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

volatile int test_flag = 9;

static void cd_trigger_null_access(void)
{
    test_flag++;

    volatile rt_uint32_t value = *(volatile rt_uint32_t *)RT_NULL;

    rt_kprintf("value: 0x%08x\n", value);
}

static void cd_invoke_fault_chain(void)
{
    cd_trigger_null_access();
}

static void coredump_test_start(void)
{
    rt_thread_t thread = rt_thread_self();
    if (thread == RT_NULL)
    {
        rt_kprintf("current thread is null\n");
        return;
    }

    cd_invoke_fault_chain();
}
MSH_CMD_EXPORT(
    coredump_test_start, Trigger a coredump test null pointer access);
