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
#include <rtdevice.h>
#include "rt_perf.h"
#include "rt_perf_testcase.h"
#include <stdlib.h>

static rt_device_t timer = RT_NULL;
volatile static rt_bool_t complete = RT_FALSE;
volatile static rt_bool_t flag = RT_FALSE;

static void jitter_get(struct rt_perf *perf)
{
    perf->real_time = (rt_uint32_t)abs(
        (perf->timer_frq / RT_TICK_PER_SECOND) - perf->real_time);
}

static rt_err_t tmr_callbak(rt_device_t dev, rt_size_t size)
{
    flag = RT_TRUE;
    return RT_EOK;
}

static void perf_thread_jitter(void *parameter)
{
    struct rt_perf *perf = (struct rt_perf *)parameter;

    rt_thread_delay(1);

    while (1)
    {
        /* wait timer trigger */
        while (!flag);

        if (perf->begin_time)
        {
            rt_perf_stop(perf);

            if (perf->count >= PERF_COUNT)
            {
                rt_device_close(timer);
                flag = RT_FALSE;
                complete = RT_TRUE;

                return;
            }
        }

        rt_perf_start(perf);
        flag = RT_FALSE;
    }
}

static rt_err_t hwtimer_init(const char *name, uint32_t us)
{
    rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;
    rt_hwtimerval_t timeout = { 0 };

    timer = rt_device_find(name);
    if (timer == RT_NULL)
    {
        rt_kprintf("Cannot find timer device: %s", name);
        return -RT_ERROR;
    }

    rt_device_open(timer, RT_DEVICE_FLAG_RDWR);
    rt_device_set_rx_indicate(timer, tmr_callbak);
    rt_device_control(timer, HWTIMER_CTRL_MODE_SET, (void *)&mode);

    timeout.sec = us / 1000000;
    timeout.usec = us % 1000000;
    if (rt_device_write(timer, 0, &timeout, sizeof(timeout)) == 0)
    {
        rt_kprintf("timer write failed");
    }

    return RT_EOK;
}

rt_err_t rt_perf_thread_jitter(struct rt_perf *perf)
{
    rt_thread_t thread = RT_NULL;
    rt_err_t ret = RT_EOK;
    complete = RT_FALSE;

    thread = rt_thread_create("perf_thread_jitter", perf_thread_jitter, perf,
        THREAD_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread == RT_NULL)
    {
        rt_kprintf("perf_thread_jitter create failed.\n");
        return (-RT_ERROR);
    }

    rt_perf_manual_set(perf, jitter_get);

    rt_thread_control(thread, RT_THREAD_CTRL_BIND_CPU, (void *)PERF_CPU);
    rt_thread_startup(thread);

    ret = hwtimer_init(RT_PERF_USING_TIMER_NAME, 1000);
    if (ret != RT_EOK)
    {
        rt_kprintf("init timer failed.\n");
        return (-RT_ERROR);
    }

    while (!complete) { rt_thread_mdelay(100); }

    rt_perf_dump(perf);

    return RT_EOK;
}
