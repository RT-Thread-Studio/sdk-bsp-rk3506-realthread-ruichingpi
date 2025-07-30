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

#define OUTPUT_GPIO 33
#define INPUT_GPIO  32

rt_err_t gpio_input_example(void)
{
    rt_uint8_t value = 0;
    rt_uint32_t count = 10;

    rt_pin_mode(OUTPUT_GPIO, PIN_MODE_OUTPUT);
    rt_pin_mode(INPUT_GPIO, PIN_MODE_INPUT);

    LOG_I("gpio input startup");

    while (count--)
    {
        rt_pin_write(OUTPUT_GPIO, PIN_HIGH);
        value = rt_pin_read(INPUT_GPIO);
        LOG_I("input gpio %d level is %s", INPUT_GPIO,
            (value) ? ("high") : ("low"));
        rt_thread_mdelay(1000);

        rt_pin_write(OUTPUT_GPIO, PIN_LOW);
        value = rt_pin_read(INPUT_GPIO);
        LOG_I("input gpio %d level is %s", INPUT_GPIO,
            (value) ? ("high") : ("low"));
        rt_thread_mdelay(1000);
    }

    LOG_I("gpio input end");

    return RT_EOK;
}
MSH_CMD_EXPORT(gpio_input_example, gpio input exampl);
