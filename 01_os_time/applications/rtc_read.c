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

#define DBG_TAG "example.rtc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_err_t rtc_example(void)
{
    rt_err_t ret;
    rt_uint8_t count = 10;
    time_t now = { 0 };

    ret = set_date(2025, 12, 31);
    if (ret != RT_EOK)
    {
        LOG_E("set RTC date failed");
        return ret;
    }

    ret = set_time(23, 59, 56);
    if (ret != RT_EOK)
    {
        LOG_E("set RTC time failed");
        return ret;
    }

    while (count--)
    {
        now = time(RT_NULL);
        char *time_str = ctime(&now);
        time_str[strcspn(time_str, "\n")] = '\0';
        LOG_I("now time: %s", time_str);
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(rtc_example, read rtc time example);
