/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <drv_ad7606.h>

// #define DRV_DEBUG
#define DBG_TAG "drv.ad7606"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_WARNING
#endif /* DRV_DEBUG */
#include <rtdbg.h>

static rt_sem_t ad_sem = RT_NULL;

static void ad7606_set_os(struct ad7606_device *ad_dev, rt_uint8_t ad_os)
{
    switch (ad_os)
    {
    case AD_OS_X2:
        rt_pin_write(ad_dev->os2_pin, PIN_LOW);
        rt_pin_write(ad_dev->os1_pin, PIN_LOW);
        rt_pin_write(ad_dev->os0_pin, PIN_HIGH);
        break;
    case AD_OS_X4:
        rt_pin_write(ad_dev->os2_pin, PIN_LOW);
        rt_pin_write(ad_dev->os1_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os0_pin, PIN_LOW);
        break;
    case AD_OS_X8:
        rt_pin_write(ad_dev->os2_pin, PIN_LOW);
        rt_pin_write(ad_dev->os1_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os0_pin, PIN_HIGH);
        break;
    case AD_OS_X16:
        rt_pin_write(ad_dev->os2_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os1_pin, PIN_LOW);
        rt_pin_write(ad_dev->os0_pin, PIN_LOW);
        break;
    case AD_OS_X32:
        rt_pin_write(ad_dev->os2_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os1_pin, PIN_LOW);
        rt_pin_write(ad_dev->os0_pin, PIN_HIGH);
        break;
    case AD_OS_X64:
        rt_pin_write(ad_dev->os2_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os1_pin, PIN_HIGH);
        rt_pin_write(ad_dev->os0_pin, PIN_LOW);
        break;
    case AD_OS_NO:
    default:
        rt_pin_write(ad_dev->os2_pin, PIN_LOW);
        rt_pin_write(ad_dev->os1_pin, PIN_LOW);
        rt_pin_write(ad_dev->os0_pin, PIN_LOW);
        break;
    }
}

static void ad7606_set_range(struct ad7606_device *ad_dev, rt_uint8_t val)
{
    rt_pin_write(ad_dev->range_pin, val ? PIN_HIGH : PIN_LOW);
}

static rt_err_t ad7606_request_gpios(
    struct rt_ofw_node *np, struct ad7606_device *ad_dev)
{
    rt_dm_dev_prop_read_u32_index(ad_dev->dev, "cs-gpios", 1, &ad_dev->cs_pin);
    if (ad_dev->cs_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: cs-gpios not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(ad_dev->dev, "rd-gpios", 1, &ad_dev->rd_pin);
    if (ad_dev->rd_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: rd-gpios not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "reset-gpios", 1, &ad_dev->rst_pin);
    if (ad_dev->rst_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: reset-gpios not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "busy-gpios", 1, &ad_dev->busy_pin);
    if (ad_dev->busy_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: busy-gpios not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "range-gpios", 1, &ad_dev->range_pin);
    if (ad_dev->range_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: range-gpios not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "adi,conversion-start-gpios-a", 1, &ad_dev->cva_pin);
    if (ad_dev->cva_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: adi,conversion-start-gpios-a not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "adi,conversion-start-gpios-b", 1, &ad_dev->cvb_pin);
    if (ad_dev->cvb_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: adi,conversion-start-gpios-b not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "adi,oversampling-ratio-gpios-0", 1, &ad_dev->os0_pin);
    if (ad_dev->os0_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: adi,oversampling-ratio-gpios-0 not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "adi,oversampling-ratio-gpios-1", 1, &ad_dev->os1_pin);
    if (ad_dev->os1_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: adi,oversampling-ratio-gpios-1 not found!\n");
        return (-RT_ERROR);
    }

    rt_dm_dev_prop_read_u32_index(
        ad_dev->dev, "adi,oversampling-ratio-gpios-2", 1, &ad_dev->os2_pin);
    if (ad_dev->os2_pin == PIN_NONE)
    {
        rt_kprintf("AD7606: adi,oversampling-ratio-gpios-2 not found!\n");
        return (-RT_ERROR);
    }

    rt_ofw_prop_read_u32(np, "adi,oversampling_ratio", &ad_dev->oversampling);

    rt_ofw_prop_read_u32(np, "adi,range", &ad_dev->range);

    return RT_EOK;
}

static rt_err_t ad7606_gpio_init(struct ad7606_device *ad_dev)
{
    rt_pin_mode(ad_dev->busy_pin, PIN_MODE_INPUT);

    rt_pin_mode(ad_dev->rst_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->cva_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->cvb_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->cs_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->rd_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->range_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->os0_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->os1_pin, PIN_MODE_OUTPUT);
    rt_pin_mode(ad_dev->os2_pin, PIN_MODE_OUTPUT);

    rt_pin_write(ad_dev->cs_pin, PIN_HIGH);
    rt_pin_write(ad_dev->rd_pin, PIN_HIGH);
    rt_pin_write(ad_dev->cva_pin, PIN_HIGH);
    rt_pin_write(ad_dev->cvb_pin, PIN_HIGH);
    rt_pin_write(ad_dev->range_pin, PIN_LOW);
    rt_pin_write(ad_dev->os0_pin, PIN_LOW);
    rt_pin_write(ad_dev->os1_pin, PIN_LOW);
    rt_pin_write(ad_dev->os2_pin, PIN_LOW);

    return RT_EOK;
}

static void ad7606_hard_reset(struct ad7606_device *ad_dev)
{
    rt_pin_write(ad_dev->rst_pin, PIN_LOW);
    rt_thread_mdelay(1);

    rt_pin_write(ad_dev->rst_pin, PIN_HIGH);
    rt_thread_mdelay(2);

    rt_pin_write(ad_dev->rst_pin, PIN_LOW);
}

static void ad7606_busy_isr(void *args)
{
    rt_sem_release(ad_sem);
}

static void ad7606_start_conv(struct ad7606_device *ad_dev)
{
    rt_pin_write(ad_dev->cva_pin, PIN_LOW);
    rt_pin_write(ad_dev->cvb_pin, PIN_LOW);

    rt_thread_mdelay(1);
    rt_pin_write(ad_dev->cva_pin, PIN_HIGH);
    rt_pin_write(ad_dev->cvb_pin, PIN_HIGH);
}

static rt_err_t ad7606_enabled(
    struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    struct ad7606_device *ad_dev;

    ad_dev = rt_container_of(device, struct ad7606_device, adc_dev);

    if (enabled)
    {
        ad7606_gpio_init(ad_dev);
        ad7606_hard_reset(ad_dev);
        ad7606_set_os(ad_dev, ad_dev->oversampling);
        ad7606_set_range(ad_dev, ad_dev->range);

        if (ad_sem == RT_NULL)
        {
            ad_sem = rt_sem_create("ad_sem", 0, RT_IPC_FLAG_FIFO);
            if (ad_sem == RT_NULL)
            {
                LOG_E("sem create fail");
                return -RT_ENOMEM;
            }
        }

        rt_adc_enable(ad_dev->fb_dev, channel);

        rt_pin_attach_irq(
            ad_dev->busy_pin, PIN_IRQ_MODE_FALLING, ad7606_busy_isr, RT_NULL);
        rt_pin_irq_enable(ad_dev->busy_pin, PIN_IRQ_ENABLE);
    }
    else
    {
        rt_pin_irq_enable(ad_dev->busy_pin, PIN_IRQ_DISABLE);
        rt_pin_detach_irq(ad_dev->busy_pin);

        rt_adc_disable(ad_dev->fb_dev, channel);

        if (ad_sem)
        {
            rt_sem_delete(ad_sem);
            ad_sem = RT_NULL;
        }

        rt_pin_write(ad_dev->cs_pin, PIN_HIGH);
        rt_pin_write(ad_dev->rd_pin, PIN_HIGH);
    }

    return RT_EOK;
}

static rt_err_t ad7606_read_raw(
    struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    struct ad7606_device *ad_dev;
    ad_dev = rt_container_of(device, struct ad7606_device, adc_dev);

    ad7606_start_conv(ad_dev);

    if (rt_sem_take(ad_sem, RT_WAITING_FOREVER) == RT_EOK)
    {
        rt_pin_write(ad_dev->cs_pin, PIN_LOW);

        for (int i = 0; i < 8; i++)
        {
            rt_pin_write(ad_dev->cs_pin, PIN_LOW);
            rt_pin_write(ad_dev->rd_pin, PIN_LOW);

            ad_dev->data[i] = rt_adc_read(ad_dev->fb_dev, i);

            rt_pin_write(ad_dev->rd_pin, PIN_HIGH);
            rt_pin_write(ad_dev->cs_pin, PIN_HIGH);
            rt_thread_mdelay(1);
        }
    }
    else
    {
        LOG_E("wait busy timeout");
        return -RT_ETIMEOUT;
    }

    *value = (rt_uint32_t)ad_dev->data[channel - 1];

    return RT_EOK;
}

static const struct rt_adc_ops ad7606_adc_ops = {
    .enabled = ad7606_enabled,
    .convert = ad7606_read_raw,
};

static rt_err_t ad7606_init(struct rt_device *dev)
{
    const char *dev_name;
    struct ad7606_device *ad7606_dev;
    struct rt_ofw_node *np = dev->ofw_node;
    rt_err_t ret;

    ad7606_dev = rt_malloc(sizeof(struct ad7606_device));
    if (!ad7606_dev)
    {
        return (-RT_ENOMEM);
    }

    ad7606_dev->dev = dev;
    ad7606_dev->adc_dev.ops = &ad7606_adc_ops;

    ad7606_dev->fb_dev = (rt_adc_device_t)rt_device_find("flexbus_adc0");
    if (ad7606_dev->fb_dev == RT_NULL)
    {
        LOG_I("flexbus adc device %s not found", "flexbus_adc0");
        return (-RT_ERROR);
    }

    rt_dm_dev_set_name_auto(dev, "adc");
    dev_name = rt_dm_dev_get_name(dev);

    ad7606_request_gpios(np, ad7606_dev);

    ret = rt_hw_adc_register(
        &ad7606_dev->adc_dev, dev_name, &ad7606_adc_ops, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("%s register fail", dev_name);
        ret = (-RT_ERROR);
    }

    return RT_EOK;
}

static rt_err_t ad7606_probe(struct rt_platform_device *pdev)
{
    if (ad7606_init(&pdev->parent))
    {
        LOG_E("ad7606 init failed");
        return (-RT_ERROR);
    }

    LOG_I("ad7606_probe");

    return RT_EOK;
}

static const struct rt_ofw_node_id ad7606_ofw_ids[] = {
    { .compatible = "adi,ad7606-8" }, { /* sentinel */ }
};

static struct rt_platform_driver ad7606_driver = {
    .name = "ad7606",
    .ids = ad7606_ofw_ids,
    .probe = ad7606_probe,
};

int ad7606_register(void)
{
    return rt_platform_driver_register(&ad7606_driver);
}
INIT_COMPONENT_EXPORT(ad7606_register);
