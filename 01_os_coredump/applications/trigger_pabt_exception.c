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

static void level3_prefetch_abort(void)
{
    void (*func)(void) = (void (*)(void))0xFF000001;
    func();
}

static void level2_prefetch_abort(void)
{
    rt_kprintf("Entering level2_prefetch_abort\n");
    level3_prefetch_abort();
    rt_kprintf("This line should not be reached\n");
}

static void level1_prefetch_abort(void)
{
    rt_kprintf("Entering level1_prefetch_abort\n");
    level2_prefetch_abort();
    rt_kprintf("This line should not be reached\n");
}

static void prefetch_abort_thread_entry(void *parameter)
{
    rt_kprintf("Starting prefetch abort test\n");
    level1_prefetch_abort();
    rt_kprintf("This line should not be reached\n");
}

void trigger_prefetch_abort_exception(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("pabt_test", prefetch_abort_thread_entry, RT_NULL,
        1024, 20, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}
MSH_CMD_EXPORT(
    trigger_prefetch_abort_exception, Trigger prefetch abort exception test);
