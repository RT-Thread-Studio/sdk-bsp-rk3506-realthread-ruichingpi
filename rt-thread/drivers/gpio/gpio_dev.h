/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __GPIO_DEV_H__
#define __GPIO_DEV_H__

#include <rtdef.h>
#include <drivers/ioctl.h>

#define PIN_NONE                    (-1)

#define PIN_LOW                     0x00
#define PIN_HIGH                    0x01

#define PIN_MODE_OUTPUT             0x00
#define PIN_MODE_INPUT              0x01
#define PIN_MODE_INPUT_PULLUP       0x02
#define PIN_MODE_INPUT_PULLDOWN     0x03
#define PIN_MODE_OUTPUT_OD          0x04

#define PIN_IRQ_MODE_RISING         0x00
#define PIN_IRQ_MODE_FALLING        0x01
#define PIN_IRQ_MODE_RISING_FALLING 0x02
#define PIN_IRQ_MODE_HIGH_LEVEL     0x03
#define PIN_IRQ_MODE_LOW_LEVEL      0x04

#define PIN_IRQ_DISABLE             0x00
#define PIN_IRQ_ENABLE              0x01

#define PIN_IRQ_PIN_NONE            PIN_NONE

#define RT_PIN_DEV_CTRL_MODE        _IO(PIN_IO_MAGIC, 0x00)
#define RT_PIN_DEV_CTRL_GET_NAME    _IOR(PIN_IO_MAGIC, 0x01, rt_uint32_t)
#define RT_PIN_DEV_CTRL_SET_HDR     _IOW(PIN_IO_MAGIC, 0x02, struct rt_pin_irq_hdr)
#define RT_PIN_DEV_CTRL_CLR_HDR     _IOW(PIN_IO_MAGIC, 0x03, rt_uint32_t)
#define RT_PIN_DEV_CTRL_ENABLE_IRQ  _IOW(PIN_IO_MAGIC, 0x04, rt_uint32_t)
#define RT_PIN_DEV_CTRL_DISABLE_IRQ _IOW(PIN_IO_MAGIC, 0x05, rt_uint32_t)
#define RT_PIN_DEV_CTRL_DEBOUNCE                                               \
    _IOW(PIN_IO_MAGIC, 0x06, struct rt_pin_debounce)

struct rt_device_pin_mode
{
    rt_uint32_t pin;
    rt_uint8_t mode;
};

struct rt_device_pin_value
{
    rt_uint32_t pin;
    rt_uint8_t value;
};

struct rt_pin_name
{
    const char *name;
    rt_base_t pin;
};

struct rt_pin_irq_hdr
{
    rt_base_t pin;
    rt_uint8_t mode;
    void (*hdr)(void *args);
    void *args;
};

struct rt_pin_debounce
{
    rt_base_t pin;
    rt_uint32_t debounce;
};

void rt_pin_mode(rt_base_t pin, rt_uint8_t mode);

void rt_pin_write(rt_base_t pin, rt_ssize_t value);

rt_ssize_t rt_pin_read(rt_base_t pin);

rt_base_t rt_pin_get(const char *name);

rt_err_t rt_pin_attach_irq(
    rt_base_t pin, rt_uint8_t mode, void (*hdr)(void *args), void *args);

rt_err_t rt_pin_detach_irq(rt_base_t pin);

rt_err_t rt_pin_irq_enable(rt_base_t pin, rt_uint8_t enabled);

rt_err_t rt_pin_debounce(rt_base_t pin, rt_uint32_t debounce);

#endif /* __GPIO_DEV_H__ */
