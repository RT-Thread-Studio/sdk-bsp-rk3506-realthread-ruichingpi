/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtdevice.h>
#include <gpio_dev.h>

static struct rt_device *gs_device = RT_NULL;

void rt_pin_mode(rt_base_t pin, rt_uint8_t mode)
{
    struct rt_device_pin_mode pin_mode;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_mode.pin = pin;
    pin_mode.mode = mode;

    rt_device_control(gs_device, RT_PIN_DEV_CTRL_MODE, &pin_mode);
}

void rt_pin_write(rt_base_t pin, rt_ssize_t value)
{
    struct rt_device_pin_value pin_value;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_value.pin = pin;
    pin_value.value = value;

    rt_device_write(
        gs_device, 0, &pin_value, sizeof(struct rt_device_pin_value));
}

rt_ssize_t rt_pin_read(rt_base_t pin)
{
    struct rt_device_pin_value pin_value;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_value.pin = pin;

    rt_device_read(
        gs_device, 0, &pin_value, sizeof(struct rt_device_pin_value));

    return pin_value.value;
}

rt_base_t rt_pin_get(const char *name)
{
    struct rt_pin_name pin_name;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_name.name = name;

    rt_device_control(gs_device, RT_PIN_DEV_CTRL_GET_NAME, &pin_name);

    return pin_name.pin;
}

rt_err_t rt_pin_attach_irq(
    rt_base_t pin, rt_uint8_t mode, void (*hdr)(void *args), void *args)
{
    struct rt_pin_irq_hdr pin_irq_hdr;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_irq_hdr.pin = pin;
    pin_irq_hdr.mode = mode;
    pin_irq_hdr.hdr = hdr;
    pin_irq_hdr.args = args;

    return rt_device_control(gs_device, RT_PIN_DEV_CTRL_SET_HDR, &pin_irq_hdr);
}

rt_err_t rt_pin_detach_irq(rt_base_t pin)
{
    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    return rt_device_control(gs_device, RT_PIN_DEV_CTRL_GET_NAME, &pin);
}

rt_err_t rt_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled)
{
    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    if (enabled)
    {
        return rt_device_control(gs_device, RT_PIN_DEV_CTRL_ENABLE_IRQ, &pin);
    }

    return rt_device_control(gs_device, RT_PIN_DEV_CTRL_DISABLE_IRQ, &pin);
}

rt_err_t rt_pin_debounce(rt_base_t pin, rt_uint32_t debounce)
{
    struct rt_pin_debounce pin_debounce;

    if (!gs_device)
    {
        gs_device = rt_device_find("gpio");
        rt_device_open(gs_device, RT_DEVICE_FLAG_RDWR);
    }

    pin_debounce.pin = pin;
    pin_debounce.debounce = debounce;

    return rt_device_control(
        gs_device, RT_PIN_DEV_CTRL_DEBOUNCE, &pin_debounce);
}
