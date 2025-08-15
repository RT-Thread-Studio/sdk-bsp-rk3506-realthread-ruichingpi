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
#ifdef COMP_USING_BACKTRACE
#include <backtrace_server.h>
#endif
#ifdef COMP_USING_COREDUMP
#include <coredump_server.h>
#endif
#include <finsh_service.h>
#include <kpi.h>

struct app_info
{
    rt_uint32_t version_ident;
};

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

    while (p < &__bss_end) { *p++ = 0; }
}

rt_section(".text.app_entrypoint") void _START(void *parameter)
{
    clean_bss();

    kpi_init();

    if(parameter == RT_NULL)
    {
        rt_kprintf("The app parameter is empty. Please update the kernel\n");
        rt_kprintf("app startup failed!\n");
        return ;
    }

    finsh_init();

#ifdef COMP_USING_BACKTRACE
    backtrace_init();
#endif

#ifdef COMP_USING_COREDUMP
    coredump_init();
#endif

    auto_init();

    extern int main(void);
    main();
}
