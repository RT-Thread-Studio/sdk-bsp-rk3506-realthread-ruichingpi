#!/usr/bin/env python3
"""Change AMP peripheral ownership in a DTS source file."""

import argparse
import re
import shutil
import sys
from pathlib import Path


def block_end(text: str, start: int) -> int:
    depth = 0
    in_string = False
    escaped = False
    for pos in range(start, len(text)):
        char = text[pos]
        if in_string:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
        elif char == '"':
            in_string = True
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return pos
    raise ValueError("unterminated node")


def find_block(text: str, selector: str) -> tuple[int, int]:
    if selector.startswith("&"):
        pattern = re.compile(r"(?m)^\s*" + re.escape(selector) + r"\s*\{")
    else:
        pattern = re.compile(r"(?m)^\s*(?:[A-Za-z_][\w-]*\s*:\s*)?" +
                             re.escape(selector) + r"\s*\{")
    match = pattern.search(text)
    if not match:
        raise ValueError(f"node not found: {selector}")
    start = text.find("{", match.start())
    try:
        return start, block_end(text, start)
    except ValueError as exc:
        raise ValueError(f"unterminated node: {selector}") from exc


def find_direct_status(body: str):
    depth = 0
    offset = 0
    prop = re.compile(r'^(\s*)status\s*=\s*"([^"]*)"\s*;')
    for line in body.splitlines(keepends=True):
        match = prop.match(line) if depth == 0 else None
        if match:
            return offset + match.start(), offset + match.end(), match.group(1), match.group(2)
        structural = re.sub(r'"(?:\\.|[^"\\])*"', '""', line.split("//", 1)[0])
        depth += structural.count("{") - structural.count("}")
        offset += len(line)
    return None


def find_direct_property(body: str, name: str):
    depth = 0
    offset = 0
    start_re = re.compile(r'^\s*' + re.escape(name) + r'\s*=')
    for line in body.splitlines(keepends=True):
        match = start_re.match(line) if depth == 0 else None
        if match:
            end = body.find(";", offset + match.start())
            if end < 0:
                raise ValueError(f"unterminated property: {name}")
            return offset + match.start(), end + 1, body[offset + match.start():end + 1]
        structural = re.sub(r'"(?:\\.|[^"\\])*"', '""', line.split("//", 1)[0])
        depth += structural.count("{") - structural.count("}")
        offset += len(line)
    return None


def included_sources(root: Path) -> list[Path]:
    result, pending = [], [root.resolve()]
    workspace = Path(__file__).resolve().parent.parent
    search_roots = [
        root.resolve().parent,
        workspace / "apps/rockchip/rk3506/platform/dtsi",
    ]
    file_index = {}
    while pending:
        path = pending.pop()
        if path in result or not path.exists():
            continue
        result.append(path)
        text = path.read_text(encoding="utf-8")
        for name in re.findall(r'^\s*#include\s+["<]([^">]+)[">]', text, re.M):
            if Path(name).suffix not in (".dts", ".dtsi"):
                continue
            candidate = path.parent / name
            if not candidate.exists():
                basename = Path(name).name
                if basename not in file_index:
                    file_index[basename] = [
                        item for directory in search_roots if directory.exists()
                        for item in directory.rglob(basename)
                    ]
                matches = file_index[basename]
                candidate = next((item for item in matches if item.as_posix().endswith(name)), None)
            if candidate is not None and candidate.exists():
                pending.append(candidate.resolve())
    return result


def find_definition(sources: list[Path], selector: str, prop_name: str):
    label = selector[1:] if selector.startswith("&") else None
    for path in sources:
        text = path.read_text(encoding="utf-8")
        patterns = [selector]
        if label:
            match = re.search(r'(?m)^\s*' + re.escape(label) + r'\s*:\s*([\w,@.+-]+)\s*\{', text)
            if match:
                patterns.append(match.group(0).split("{")[0].strip())
        for pattern in patterns:
            try:
                start, end = find_block(text, pattern)
            except ValueError:
                continue
            prop = find_direct_property(text[start + 1:end], prop_name)
            if prop:
                return prop[2]
    return None


NODE_HEADER = re.compile(
    r'(?m)^\s*((?:&[\w-]+)|(?:(?P<label>[A-Za-z_][\w-]*)\s*:\s*)?'
    r'(?P<name>[\w,@.+-]+))\s*\{')


def effective_nodes(sources: list[Path]):
    labels = {}
    for path in sources:
        text = path.read_text(encoding="utf-8")
        for header in NODE_HEADER.finditer(text):
            if header.group("label"):
                labels[header.group("name")] = "&" + header.group("label")

    nodes = {}
    # included_sources starts with the board DTS; process it last so overrides win.
    for path in reversed(sources):
        text = path.read_text(encoding="utf-8")
        for header in NODE_HEADER.finditer(text):
            raw = header.group(1).strip()
            selector = raw if raw.startswith("&") else (
                "&" + header.group("label") if header.group("label") else
                labels.get(header.group("name"), header.group("name")))
            start = header.end() - 1
            try:
                end = block_end(text, start)
            except ValueError:
                continue
            status = find_direct_status(text[start + 1:end])
            if status:
                nodes[selector] = status[3]
    return nodes


def effective_status(sources: list[Path], selector: str):
    nodes = effective_nodes(sources)
    if selector in nodes:
        return nodes[selector]
    if not selector.startswith("&") and "&" + selector in nodes:
        return nodes["&" + selector]
    if not selector.startswith("&"):
        for path in sources:
            text = path.read_text(encoding="utf-8")
            match = re.search(r'(?m)^\s*([A-Za-z_][\w-]*)\s*:\s*' +
                              re.escape(selector) + r'\s*\{', text)
            if match and "&" + match.group(1) in nodes:
                return nodes["&" + match.group(1)]
    return None


def amp_clock_specs(sources: list[Path]):
    for path in sources:
        text = path.read_text(encoding="utf-8")
        try:
            start, end = find_block(text, "rockchip_amp: rockchip-amp")
        except ValueError:
            continue
        prop = find_direct_property(text[start + 1:end], "clocks")
        if prop:
            return re.findall(r'<[^>]+>', prop[2])
    return []


def print_assignment_details(sources: list[Path], selector: str) -> None:
    affinity = find_definition(sources, selector, "interrupt-affinity")
    affinity_specs = re.findall(r'<[^>]+>', affinity or "")
    print("  interrupt-affinity: " + (" ".join(affinity_specs) if affinity_specs else "<unset>"))

    clocks = find_definition(sources, selector, "clocks")
    clock_specs = re.findall(r'<[^>]+>', clocks or "")
    normalize = lambda item: re.sub(r'\s+', ' ', item.strip())
    kept = {normalize(item) for item in amp_clock_specs(sources)}
    if not clock_specs:
        print("  clocks: <unset>")
        return
    details = [f"{item}({'rtt-kept' if normalize(item) in kept else 'linux'})"
               for item in clock_specs]
    print("  clocks: " + " ".join(details))


def set_direct_property(text: str, start: int, end: int, name: str, value: str) -> str:
    body = text[start + 1:end]
    prop = find_direct_property(body, name)
    line_start = text.rfind("\n", 0, start) + 1
    indent = re.match(r"\s*", text[line_start:start]).group(0) + "\t"
    replacement = f"{indent}{name} = {value};"
    if prop:
        body = body[:prop[0]] + replacement + body[prop[1]:]
    else:
        body = "\n" + replacement + body
    return text[:start + 1] + body + text[end:]


def update_amp_clock_source(sources: list[Path], clock_prop: str, to_rtt: bool, backup: bool) -> Path:
    specs = re.findall(r'<[^>]+>', clock_prop)
    if not specs:
        raise ValueError("device clocks property has no clock specifiers")
    for path in sources:
        text = path.read_text(encoding="utf-8")
        try:
            start, end = find_block(text, "rockchip_amp: rockchip-amp")
        except ValueError:
            continue
        body = text[start + 1:end]
        prop = find_direct_property(body, "clocks")
        if not prop:
            raise ValueError("rockchip_amp node has no clocks property")
        current = re.findall(r'<[^>]+>', prop[2])
        normalize = lambda item: re.sub(r'\s+', ' ', item.strip())
        wanted = {normalize(item) for item in specs}
        if to_rtt:
            merged = current + [item for item in specs if normalize(item) not in {normalize(cur) for cur in current}]
        else:
            merged = [item for item in current if normalize(item) not in wanted]
        value = ", ".join(merged)
        updated = set_direct_property(text, start, end, "clocks", value)
        if backup:
            shutil.copy2(path, path.with_suffix(path.suffix + ".bak"))
        path.write_text(updated, encoding="utf-8")
        return path
    raise ValueError("rockchip_amp source node not found in include tree")


def set_owner(path: Path, selector: str, owner: str, backup: bool) -> None:
    text = path.read_text(encoding="utf-8")
    sources = included_sources(path)
    clock_prop = find_definition(sources, selector, "clocks")
    if not clock_prop:
        raise ValueError(f"clocks property not found for {selector} in include tree")

    try:
        start, end = find_block(text, selector)
    except ValueError:
        if not selector.startswith("&"):
            raise
        text = text.rstrip() + f"\n\n{selector} {{\n}};\n"
        start, end = find_block(text, selector)
    body = text[start + 1:end]
    status = "okay,rtt" if owner == "rtt" else "okay"
    match = find_direct_status(body)
    if match:
        prop_start, prop_end, indent, _ = match
        body = body[:prop_start] + f'{indent}status = "{status}";' + body[prop_end:]
    else:
        line_start = text.rfind("\n", 0, start) + 1
        indent = re.match(r"\s*", text[line_start:start]).group(0) + "\t"
        body = f'\n{indent}status = "{status}";' + body
    updated = text[:start + 1] + body + text[end:]
    start, end = find_block(updated, selector)
    affinity = find_direct_property(updated[start + 1:end], "interrupt-affinity")
    affinity_text = affinity[2] if affinity else find_definition(sources, selector, "interrupt-affinity")
    irq_text = find_definition(sources, selector, "interrupts")
    irq_count = 0
    if irq_text:
        irq_count = irq_text.count("GIC_SPI")
        if not irq_count:
            irq_count = len(re.findall(r'<[^>]+>', irq_text))
    if not irq_count:
        irq_count = len(re.findall(r'&cpu(?:_rtt|[0-9]+)', affinity_text or ""))
    if irq_count:
        cpu = "&cpu_rtt" if owner == "rtt" else "&cpu0"
        updated = set_direct_property(updated, start, end, "interrupt-affinity",
                                      ", ".join(f"<{cpu}>" for _ in range(irq_count)))
    if backup:
        shutil.copy2(path, path.with_suffix(path.suffix + ".bak"))
    path.write_text(updated, encoding="utf-8")
    amp_path = update_amp_clock_source(sources, clock_prop, owner == "rtt", backup)
    print(f"{selector}: owner={owner}, source={path}, amp-clocks={amp_path}")


def main() -> int:
    parser = argparse.ArgumentParser(prog="amp_device_pc")
    parser.add_argument("dts", type=Path)
    parser.add_argument("command", choices=("list", "get", "set"))
    parser.add_argument("node", nargs="?", help="DTS reference such as &uart2 or node name such as uart@ff1a0000")
    parser.add_argument("owner", nargs="?", choices=("linux", "rtt"))
    parser.add_argument("--no-backup", action="store_true")
    args = parser.parse_args()
    try:
        text = args.dts.read_text(encoding="utf-8")
        sources = included_sources(args.dts)
        if args.command == "list":
            print(f'{"OWNER":7} NODE')
            for selector, status in effective_nodes(sources).items():
                if status in ("ok", "okay", "okay,rtt"):
                    owner = "rtt" if status == "okay,rtt" else "linux"
                    print(f"{owner:7} {selector}")
            return 0
        if not args.node:
            parser.error(f"{args.command} requires a node")
        if not args.node.startswith("&") and "&" + args.node in effective_nodes(sources):
            args.node = "&" + args.node
        if args.command == "get":
            value = effective_status(sources, args.node) or "<unset>"
            owner = "rtt" if value == "okay,rtt" else "linux" if value in ("ok", "okay") else "unassigned"
            print(f"{args.node}: owner={owner}, status={value}")
            print_assignment_details(sources, args.node)
        elif not args.owner:
            parser.error("set requires owner: linux or rtt")
        else:
            set_owner(args.dts, args.node, args.owner, not args.no_backup)
    except (OSError, ValueError) as exc:
        print(f"amp_device_pc: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
