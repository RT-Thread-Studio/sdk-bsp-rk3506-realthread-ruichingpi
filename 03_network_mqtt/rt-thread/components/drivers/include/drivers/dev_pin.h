/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DEV_PIN_H__
#define __DEV_PIN_H__

#include <rtthread.h>
#include <kpi.h>
#ifdef RT_USING_DM
#include <drivers/pic.h>
#endif /* RT_USING_DM */

#define PIN_NONE                    (-1)

#define PIN_LOW                     0x00 /*!< low level */
#define PIN_HIGH                    0x01 /*!< high level */

#define PIN_MODE_OUTPUT             0x00 /*!< output mode */
#define PIN_MODE_INPUT              0x01 /*!< input mode */
#define PIN_MODE_INPUT_PULLUP       0x02 /*!< input mode with pull-up */
#define PIN_MODE_INPUT_PULLDOWN     0x03 /*!< input mode with pull-down */
#define PIN_MODE_OUTPUT_OD          0x04 /*!< output mode with open-drain */

#define PIN_IRQ_MODE_RISING         0x00 /*!< rising edge trigger */
#define PIN_IRQ_MODE_FALLING        0x01 /*!< falling edge trigger */
#define PIN_IRQ_MODE_RISING_FALLING 0x02 /*!< rising and falling edge trigger */
#define PIN_IRQ_MODE_HIGH_LEVEL     0x03 /*!< high level trigger */
#define PIN_IRQ_MODE_LOW_LEVEL      0x04 /*!< low level trigger */

#define PIN_IRQ_DISABLE             0x00 /*!< disable irq */
#define PIN_IRQ_ENABLE              0x01 /*!< enable irq */

#define PIN_IRQ_PIN_NONE            PIN_NONE /*!< no pin irq */

#if 0
#ifdef RT_USING_DM
struct rt_pin_irqchip
{
    struct rt_pic parent;

    int irq;
    rt_base_t pin_range[2];
};
struct rt_pin_irq_hdr;
#endif /* RT_USING_DM */

struct rt_device_pin
{
    struct rt_device parent;
#ifdef RT_USING_DM
    /* MUST keep the order member after parent */
    struct rt_pin_irqchip irqchip;
    /* Fill by DM */
    rt_base_t pin_start;
    rt_size_t pin_nr;
    rt_list_t list;
    struct rt_pin_irq_hdr *legacy_isr;
#endif /* RT_USING_DM */
    const struct rt_pin_ops *ops;
};

struct rt_device_pin_mode
{
    rt_base_t pin;
    rt_uint8_t mode; /* e.g. PIN_MODE_OUTPUT */
};

struct rt_device_pin_value
{
    rt_base_t pin;
    rt_uint8_t value; /* PIN_LOW or PIN_HIGH */
};

struct rt_pin_irq_hdr
{
    rt_base_t pin;
    rt_uint8_t mode; /* e.g. PIN_IRQ_MODE_RISING */
    void (*hdr)(void *args);
    void *args;
};
#endif

struct rt_pin_ops
{
    void (*pin_mode)(struct rt_device *device, rt_base_t pin, rt_uint8_t mode);
    void (*pin_write)(
        struct rt_device *device, rt_base_t pin, rt_uint8_t value);
    rt_ssize_t (*pin_read)(struct rt_device *device, rt_base_t pin);
    rt_err_t (*pin_attach_irq)(struct rt_device *device,
        rt_base_t pin,
        rt_uint8_t mode,
        void (*hdr)(void *args),
        void *args);
    rt_err_t (*pin_detach_irq)(struct rt_device *device, rt_base_t pin);
    rt_err_t (*pin_irq_enable)(
        struct rt_device *device, rt_base_t pin, rt_uint8_t enabled);
    rt_base_t (*pin_get)(const char *name);
    rt_err_t (*pin_debounce)(
        struct rt_device *device, rt_base_t pin, rt_uint32_t debounce);
#ifdef RT_USING_DM
    rt_err_t (*pin_irq_mode)(
        struct rt_device *device, rt_base_t pin, rt_uint8_t mode);
    rt_ssize_t (*pin_parse)(struct rt_device *device,
        struct rt_ofw_cell_args *args,
        rt_uint32_t *flags);
#endif /* RT_USING_DM */
};

typedef int (*__kpi_rt_device_pin_register)(
    const char *name, const struct rt_pin_ops *ops, void *user_data);
typedef void (*__kpi_rt_pin_mode)(rt_base_t pin, rt_uint8_t mode);
typedef void (*__kpi_rt_pin_write)(rt_base_t pin, rt_ssize_t value);
typedef rt_ssize_t (*__kpi_rt_pin_read)(rt_base_t pin);
typedef rt_base_t (*__kpi_rt_pin_get)(const char *name);
typedef rt_err_t (*__kpi_rt_pin_attach_irq)(
    rt_base_t pin, rt_uint8_t mode, void (*hdr)(void *args), void *args);
typedef rt_err_t (*__kpi_rt_pin_detach_irq)(rt_base_t pin);
typedef rt_err_t (*__kpi_rt_pin_irq_enable)(rt_base_t pin, rt_uint8_t enabled);
typedef rt_err_t (*__kpi_rt_pin_debounce)(rt_base_t pin, rt_uint32_t debounce);

#ifdef RT_USING_DM
typedef rt_ssize_t (*__kpi_rt_pin_get_named_pin)(struct rt_device *dev,
    const char *propname,
    int index,
    rt_uint8_t *out_mode,
    rt_uint8_t *out_value);
typedef rt_ssize_t (*__kpi_rt_pin_get_named_pin_count)(
    struct rt_device *dev, const char *propname);

#ifdef RT_USING_OFW
typedef rt_ssize_t (*__kpi_rt_ofw_get_named_pin)(struct rt_ofw_node *np,
    const char *propname,
    int index,
    rt_uint8_t *out_mode,
    rt_uint8_t *out_value);
typedef rt_ssize_t (*__kpi_rt_ofw_get_named_pin_count)(
    struct rt_ofw_node *np, const char *propname);
#endif /* RT_USING_OFW */
#endif /* RT_USING_DM */

KPI_EXTERN(rt_device_pin_register);
KPI_EXTERN(rt_pin_mode);
KPI_EXTERN(rt_pin_write);
KPI_EXTERN(rt_pin_read);
KPI_EXTERN(rt_pin_get);
KPI_EXTERN(rt_pin_attach_irq);
KPI_EXTERN(rt_pin_detach_irq);
KPI_EXTERN(rt_pin_irq_enable);
KPI_EXTERN(rt_pin_debounce);
KPI_EXTERN(rt_pin_get_named_pin);
KPI_EXTERN(rt_pin_get_named_pin_count);
KPI_EXTERN(rt_ofw_get_named_pin);
KPI_EXTERN(rt_ofw_get_named_pin_count);

#endif /* __DEV_PIN_H__ */
