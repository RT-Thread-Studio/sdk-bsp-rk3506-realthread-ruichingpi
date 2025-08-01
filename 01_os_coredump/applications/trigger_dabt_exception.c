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

static void level3_data_abort(void)
{
    volatile rt_uint32_t ptr = *(volatile rt_uint32_t *)0xFF000001;

    rt_kprintf("value 0x%08x\n", ptr);
}

static void level2_data_abort(void)
{
    rt_kprintf("Entering level2_data_abort\n");
    level3_data_abort();
    rt_kprintf("This line should not be reached\n");
}

static void level1_data_abort(void)
{
    rt_kprintf("Entering level1_data_abort\n");
    level2_data_abort();
    rt_kprintf("This line should not be reached\n");
}

static void data_abort_thread_entry(void *parameter)
{
    rt_kprintf("Starting data abort test\n");
    level1_data_abort();
    rt_kprintf("This line should not be reached\n");
}

void trigger_data_abort_exception(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("data_abort_test", data_abort_thread_entry, RT_NULL,
        1024, 20, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}
MSH_CMD_EXPORT(trigger_data_abort_exception, Trigger data abort exception test);
