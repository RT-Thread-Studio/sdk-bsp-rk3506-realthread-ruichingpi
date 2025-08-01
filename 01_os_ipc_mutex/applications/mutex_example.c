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

#define THREAD_PRIORITY   10
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

static rt_thread_t mu_tid1 = RT_NULL;
static rt_thread_t mu_tid2 = RT_NULL;
static rt_thread_t mu_tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;

static void thread1_entry(void *parameter)
{
    rt_thread_mdelay(100);

    if (RT_SCHED_PRIV(mu_tid2).current_priority !=
        RT_SCHED_PRIV(mu_tid3).current_priority)
    {
        rt_kprintf("the priority of thread2 is: %d\n",
            RT_SCHED_PRIV(mu_tid2).current_priority);
        rt_kprintf("the priority of thread3 is: %d\n",
            RT_SCHED_PRIV(mu_tid3).current_priority);
        rt_kprintf("test failed.\n");
        return;
    }
    else
    {
        rt_kprintf("the priority of thread2 is: %d\n",
            RT_SCHED_PRIV(mu_tid2).current_priority);
        rt_kprintf("the priority of thread3 is: %d\n",
            RT_SCHED_PRIV(mu_tid3).current_priority);
        rt_kprintf("test OK.\n");
    }
}

static void thread2_entry(void *parameter)
{
    rt_err_t result;

    rt_thread_mdelay(5);
    rt_kprintf("the priority of thread2 is: %d\n",
        RT_SCHED_PRIV(mu_tid2).current_priority);

    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);

    if (result == RT_EOK)
    {
        rt_mutex_release(mutex);
    }
}

static void thread3_entry(void *parameter)
{
    rt_tick_t tick;
    rt_err_t result;

    rt_kprintf("the priority of thread3 is: %d\n",
        RT_SCHED_PRIV(mu_tid3).current_priority);

    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_kprintf("thread3 take a mutex, failed.\n");
    }

    tick = rt_tick_get();
    while ((rt_tick_get() - tick) < (RT_TICK_PER_SECOND / 2));

    rt_mutex_release(mutex);
}

int mutex_pri_inversion_example(void)
{
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_PRIO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    mu_tid1 = rt_thread_create("thread1", thread1_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (mu_tid1 != RT_NULL)
    {
        rt_thread_startup(mu_tid1);
    }

    mu_tid2 = rt_thread_create("thread2", thread2_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (mu_tid2 != RT_NULL)
    {
        rt_thread_startup(mu_tid2);
    }

    mu_tid3 = rt_thread_create("thread3", thread3_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (mu_tid3 != RT_NULL)
    {
        rt_thread_startup(mu_tid3);
    }

    return 0;
}
MSH_CMD_EXPORT(mutex_pri_inversion_example, mutex_pri_inversion_example);
