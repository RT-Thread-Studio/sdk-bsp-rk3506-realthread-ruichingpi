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

#define DBG_TAG "example.wdt"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_device_t device = RT_NULL;
static rt_uint32_t timeout = 11; /* timeout 11s */

static int wdt_example(void)
{
    rt_uint32_t cnt = 0;

    device = rt_device_find("wdt0");
    if (device == RT_NULL)
    {
        LOG_E("find wdt failed!");
        return (-RT_ERROR);
    }

    rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    rt_device_control(device, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    rt_device_control(device, RT_DEVICE_CTRL_WDT_START, RT_NULL);

    while (cnt < 6)
    {
        cnt++;
        rt_device_control(device, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
        LOG_I(
            "feed watchdog, current ostick is %dms", rt_tick_get_millisecond());

        rt_thread_mdelay(10000);
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(wdt_example, watchdog example);
