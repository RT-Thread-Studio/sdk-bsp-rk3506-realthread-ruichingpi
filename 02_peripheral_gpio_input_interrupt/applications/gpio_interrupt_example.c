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

static void gpio_isr(void *args)
{
    LOG_I("input gpio %d irq trigger, level is %d", INPUT_GPIO,
        rt_pin_read((INPUT_GPIO)));
}

rt_err_t gpio_interrupt_example(void)
{
    rt_uint32_t count = 10;

    rt_pin_mode(OUTPUT_GPIO, PIN_MODE_OUTPUT);
    rt_pin_mode(INPUT_GPIO, PIN_MODE_INPUT);

    rt_pin_attach_irq(
        INPUT_GPIO, PIN_IRQ_MODE_RISING_FALLING, gpio_isr, RT_NULL);
    rt_pin_irq_enable(INPUT_GPIO, PIN_IRQ_ENABLE);

    LOG_I("gpio interrupt startup");

    while (count--)
    {
        rt_pin_write(OUTPUT_GPIO, PIN_HIGH);
        rt_thread_mdelay(500);

        rt_pin_write(OUTPUT_GPIO, PIN_LOW);
        rt_thread_mdelay(500);
    }

    rt_pin_detach_irq(INPUT_GPIO);
    rt_pin_irq_enable(INPUT_GPIO, PIN_IRQ_DISABLE);

    LOG_I("gpio interrupt end");

    return RT_EOK;
}
MSH_CMD_EXPORT(gpio_interrupt_example, gpio interrupt example);
