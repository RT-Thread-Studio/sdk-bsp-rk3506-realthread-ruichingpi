/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RTM_H__
#define __RTM_H__

#include <rtdef.h>
#include <rtthread.h>

#ifdef RT_USING_MODULE
struct rt_module_symtab
{
    void       *addr;
    const char *name;
};

#if defined(_MSC_VER)
#pragma section("RTMSymTab$f",read)
#define RTM_EXPORT(symbol)                                            \
__declspec(allocate("RTMSymTab$f"))const char __rtmsym_##symbol##_name[] = "__vs_rtm_"#symbol;
#pragma comment(linker, "/merge:RTMSymTab=mytext")

#elif defined(__MINGW32__)
#define RTM_EXPORT(symbol)

#else
#define RTM_EXPORT(symbol)                                            \
const char __rtmsym_##symbol##_name[] rt_section(".rodata.name") = #symbol;     \
const struct rt_module_symtab __rtmsym_##symbol rt_section("RTMSymTab")= \
{                                                                     \
    (void *)&symbol,                                                  \
    __rtmsym_##symbol##_name                                          \
};
#endif

#else
#define RTM_EXPORT(symbol)
#endif

#endif
