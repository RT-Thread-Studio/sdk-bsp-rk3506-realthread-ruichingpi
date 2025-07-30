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

#define DBG_TAG "example.gpio"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define LED 160

rt_err_t gpio_output_example(void)
{
    rt_uint32_t count = 10;

    rt_pin_mode(LED, PIN_MODE_OUTPUT);

    LOG_I("gpio output startup");

    while (count--)
    {
        rt_pin_write(LED, PIN_HIGH);
        rt_thread_mdelay(1000);

        rt_pin_write(LED, PIN_LOW);
        rt_thread_mdelay(1000);
    }

    LOG_I("gpio output end");

    return RT_EOK;
}
MSH_CMD_EXPORT(gpio_output_example, gpio output example);
