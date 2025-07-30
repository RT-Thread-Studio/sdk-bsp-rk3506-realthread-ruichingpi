/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include <kpi.h>
#include <finsh_service.h>
#include <backtrace_server.h>
#include <coredump_server.h>

static int rti_start(void)
{
    return 0;
}
INIT_EXPORT(rti_start, "0");

static int rti_end(void)
{
    return 0;
}
INIT_EXPORT(rti_end, "6.end");

static void auto_init(void)
{
    volatile const init_fn_t *fn_ptr;

    for (fn_ptr = &__rt_init_rti_start; fn_ptr < &__rt_init_rti_end; fn_ptr++)
    {
        (*fn_ptr)();
    }
}

void clean_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;

    while (p < &__bss_end)
    {
        *p++ = 0;
    }
}

rt_section(".text.app_entrypoint") void _START(void)
{
    clean_bss();

    kpi_init();

    finsh_init();

    backtrace_init();

    coredump_init();

    auto_init();

    extern int main(void);
    main();
}
