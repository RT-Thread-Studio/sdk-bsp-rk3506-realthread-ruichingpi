/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

MEMORY
{
    KPI (rxw) : ORIGIN = 0x05000000, LENGTH = 0x4000
    RAM (rxw) : ORIGIN = 0x04000000 + 0x40, LENGTH = 0x1000000
}

SECTIONS
{
    . = ALIGN(16);
    .kpi :
    {
        __kpi_start = .;
        . += 0x4000;
        __kpi_end = .;
    } > KPI

    .text :
    {
        __text_start = .;
        KEEP(*(.text.app_entrypoint))
        *(.text)
        *(.rodata)
        *(.rodata*)

        /* section information for initialization */
        . = ALIGN(16);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;

        . = ALIGN(16);
        __app_fsymtab_start = .;
        KEEP(*(FSymTab))
        __app_fsymtab_end = .;

        __text_end = .;
    } > RAM

    . = ALIGN(16);
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > RAM
    
    . = ALIGN(16);
    .ctors :
    {
        PROVIDE(__ctors_start__ = .);
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        PROVIDE(__ctors_end__ = .);
    } > RAM
    . = ALIGN(16);

    . = ALIGN(16);
    .data :
    {
        __data_start = .;
        *(.data)
        *(.data.*)
        __data_end = .;

        *(.sdata)
        *(.sdata.*)
    } > RAM

    . = ALIGN(16);
    .bss :
    {
        PROVIDE(__bss_start = .);
        __bss_cd_start = .;
        *(.bss)
        __bss_cd_end = .;

        *(.bss.*)
        *(.dynbss)
        PROVIDE(__bss_end = .);
    } > RAM
    _end = .;

    .stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug 0 : { *(.debug) }
    .line 0 : { *(.line) }
    .debug_srcinfo 0 : { *(.debug_srcinfo) }
    .debug_sfnames 0 : { *(.debug_sfnames) }
    .debug_aranges 0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line 0 : { *(.debug_line) }
    .debug_frame 0 : { *(.debug_frame) }
    .debug_str 0 : { *(.debug_str) }
    .debug_loc 0 : { *(.debug_loc) }
    .debug_macinfo 0 : { *(.debug_macinfo) }
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames 0 : { *(.debug_varnames) }
    __data_size = SIZEOF(.data);
    __bss_size = SIZEOF(.bss);
}
