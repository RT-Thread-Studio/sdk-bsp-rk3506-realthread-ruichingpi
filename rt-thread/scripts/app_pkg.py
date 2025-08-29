# -*- coding: utf-8 -*-
from __future__ import print_function  # 确保 print 在 Python 2 和 3 中都是函数
import os
import struct
import zlib
import re
import sys

MAGIC = struct.unpack("<I", b"app\0")[0]  # 使用 b"" 定义字节串
LOAD_ADDR = 0x4000000 + 0x40
APP_SIZE = 0x1000000  # 16M
VERSION_IDENT = 0x11112223

def parse_map_for_entrypoint(map_file):
    with open(map_file, "r") as f:
        content = f.read()

    match = re.search(r"(\b0x[0-9a-fA-F]+)\s+_START", content)
    if match:
        return int(match.group(1), 16)

    raise ValueError("not find .text.app_entrypoint")

def compute_crc32(file_path):
    with open(file_path, "rb") as f:
        data = f.read()
    return zlib.crc32(data) & 0xFFFFFFFF  # 确保结果是无符号的 32 位整数

def pack_bin(build_path, debug_mode=0):
    bin_file = os.path.join(build_path, 'app.bin')
    map_file = os.path.join(build_path, 'app.map')
    if debug_mode:
        out_file = os.path.join(build_path, 'app_debug.img')
    else:
        out_file = os.path.join(build_path, 'app.img')

    start_addr = parse_map_for_entrypoint(map_file)

    with open(bin_file, "rb") as f:
        bin_data = f.read()

    crc_value = compute_crc32(bin_file)

    size = os.path.getsize(bin_file)

    # 定义 8 个保留字段，初始化为 0，并添加 crc_value
    res = [0] * 7 + [crc_value]

    # 按照结构体 smodule_head 的排布打包头信息
    header = struct.pack("<I I I I I 11I",
                         MAGIC,          # magic
                         LOAD_ADDR,      # load_addr
                         start_addr,     # start_addr
                         size,           # size
                         1,              # update
                         0,              # download
                         debug_mode,     # debug
                         VERSION_IDENT,  # 版本标识符
                         *res)           # res[7] + crc_value

    with open(out_file, "wb") as f:
        f.write(header)
        f.write(bin_data)

    print("%s firmware packaged success" %out_file)
    print("info:")
    print("  {:<10}: 0x{:08X}".format("start_addr", start_addr))
    print("  {:<10}: 0x{:08X}".format("load_addr", LOAD_ADDR))
    print("  {:<10}: 0x{:08X}".format("size", size))
