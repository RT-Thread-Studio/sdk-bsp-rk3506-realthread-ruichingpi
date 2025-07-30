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
#include "drivers/hwtimer.h"
#include "rt_perf.h"
#include "rt_perf_testcase.h"

#define TIMER_TIMEOUT 1000

volatile static rt_bool_t complete = RT_FALSE;
static struct rt_perf *irq_perf = RT_NULL;
rt_device_t timer_dev = RT_NULL;
rt_hwtimerval_t timeout = { 0 };

static void latency_get(struct rt_perf *perf)
{
    perf->real_time =
        (rt_uint32_t)(perf->real_time - (perf->timer_frq / RT_TICK_PER_SECOND));
}

static rt_err_t timer_callback(rt_device_t dev, rt_size_t size)
{
    if (irq_perf->begin_time != 0)
    {
        rt_perf_stop(irq_perf);
        rt_device_control(dev, HWTIMER_CTRL_STOP, RT_NULL);

        if (irq_perf->count >= PERF_COUNT)
        {
            complete = RT_TRUE;
            return RT_EOK;
        }
    }

    rt_device_write(timer_dev, 0, &timeout, sizeof(timeout));
    rt_perf_start(irq_perf);

    return RT_EOK;
}

rt_err_t rt_perf_irq_latency(struct rt_perf *perf)
{
    rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;

    complete = RT_FALSE;
    irq_perf = perf;

    timer_dev = rt_device_find(RT_PERF_USING_TIMER_NAME);
    if (timer_dev == RT_NULL)
    {
        rt_kprintf("timer_dev can't find.\n");
        return (-RT_ERROR);
    }

    rt_device_open(timer_dev, RT_DEVICE_FLAG_RDWR);
    rt_device_set_rx_indicate(timer_dev, timer_callback);
    rt_device_control(timer_dev, HWTIMER_CTRL_MODE_SET, (void *)&mode);

    timeout.sec = 0;
    timeout.usec = TIMER_TIMEOUT;
    rt_device_write(timer_dev, 0, &timeout, sizeof(timeout));

    rt_perf_manual_set(perf, latency_get);

    while (!complete) { rt_thread_mdelay(10); }
    rt_perf_dump(irq_perf);

    return RT_EOK;
}
