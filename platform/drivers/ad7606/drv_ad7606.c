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

#define EVENT_DMA_DONE               (1 << 0)

static const uint32_t AD7606_SampleFreq[] = {
    200000, /* No oversampling: 200Ksps  */
    100000, /* 2x oversampling: 100Ksps */
    50000,  /* 4x oversampling: 50Ksps  */
    25000,  /* 8x oversampling: 25Ksps  */
    12500,  /* 16x oversampling: 12.5Ksps  */
    6250,   /* 32x oversampling: 6.25Ksps  */
    3125,   /* 64x oversampling: 3.125Ksps */
};

static rt_event_t _ad7606_dma_event = RT_NULL;

static rt_err_t ad7606_hwtimer_timeout(rt_device_t dev, rt_size_t size)
{
    struct ad7606_device *ad_dev = (struct ad7606_device *)dev->user_data;

    if (ad_dev && ad_dev->enabled)
    {
        rt_pin_write(ad_dev->cva_pin, PIN_LOW);
        rt_pin_write(ad_dev->cvb_pin, PIN_LOW);
        rt_hw_us_delay(1);
        rt_pin_write(ad_dev->cva_pin, PIN_HIGH);
        rt_pin_write(ad_dev->cvb_pin, PIN_HIGH);
    }

    return RT_EOK;
}

static void ad7606_busy_isr(void *args)
{
    struct ad7606_device *ad_dev = (struct ad7606_device *)args;

    if (!ad_dev || !ad_dev->enabled)
    {
        return;
    }

    if (ad_dev->data_sem)
    {
        rt_sem_release(ad_dev->data_sem);
    }
}

static void ad7606_dma_isr(struct rt_dma_chan *chan, rt_size_t size)
{
    if (_ad7606_dma_event != RT_NULL)
    {
        rt_event_send(_ad7606_dma_event, EVENT_DMA_DONE);
    }
}

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

static void ad7606_sample_entry(void *parameter)
{
    struct ad7606_device *ad_dev = (struct ad7606_device *)parameter;
    rt_tick_t timeout = rt_tick_from_millisecond(100);
    rt_uint32_t dma_index = 0;
    rt_err_t ret = RT_EOK;
    struct rt_dma_slave_transfer dma_trans = {0};

    while (ad_dev->sample_run)
    {
        if (rt_sem_take(ad_dev->data_sem, timeout) == RT_EOK)
        {
            if (!ad_dev->sample_run)
            {
                break;
            }

            if (!ad_dev->enabled)
            {
                continue;
            }

            rt_pin_write(ad_dev->cs_pin, PIN_LOW);
            for (int i = 0; i < AD7606_MAX_CHANNELS; i++)
            {
                rt_pin_write(ad_dev->rd_pin, PIN_LOW);
                ad_dev->data[i] = (rt_uint16_t)rt_adc_read(ad_dev->fb_dev, i);

                rt_pin_write(ad_dev->rd_pin, PIN_HIGH);

                if (ad_dev->dma_enable == RT_TRUE && ad_dev->dma_data != RT_NULL)
                {
                    if (ad_dev->dma_cfg.use_channels & AD7606_CHANNEL(i + 1))
                    {
                        ad_dev->dma_data[dma_index++] = (rt_uint16_t)ad_dev->data[i];
                    }
                    if (dma_index >= (ad_dev->dma_cfg.buf_len / sizeof(rt_uint16_t)))
                    {
                        dma_index = 0;
                    }
                }
            }
            rt_pin_write(ad_dev->cs_pin, PIN_HIGH);

            ad_dev->data_ready = RT_TRUE;

            if (ad_dev->dma_enable == RT_TRUE && dma_index == 0 && ad_dev->dma_data != RT_NULL)
            {
                dma_trans.src_addr = (rt_ubase_t)ad_dev->dma_data;
                dma_trans.dst_addr = (rt_ubase_t)ad_dev->dma_cfg.dst_addr;
                dma_trans.buffer_len = ad_dev->dma_cfg.buf_len;
                ret = rt_dma_prep_memcpy(ad_dev->dma_chan, &dma_trans);
                if (ret != RT_EOK)
                {
                    LOG_E("DMA prep memcpy failed");
                    continue;
                }

                rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH,
                                    (void*)ad_dev->dma_data,
                                    dma_trans.buffer_len);
                rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE,
                                    (void*)ad_dev->dma_cfg.dst_addr,
                                    dma_trans.buffer_len);

                ret = rt_dma_chan_start(ad_dev->dma_chan);
                if (ret != RT_EOK)
                {
                    LOG_E("DMA start failed");
                    continue;
                }

                ret = rt_event_recv(_ad7606_dma_event, EVENT_DMA_DONE,
                                    RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, timeout, RT_NULL);
                if (ret != RT_EOK)
                {
                    LOG_E("wait event fail: %d\n", ret);
                    continue;
                }

                rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE,
                                    (void*)ad_dev->dma_cfg.dst_addr,
                                    dma_trans.buffer_len);
            }
        }
        else
        {
            if (!ad_dev->sample_run)
            {
                break;
            }
        }
    }

    if (ad_dev->dma_chan != RT_NULL)
    {
        rt_dma_chan_release(ad_dev->dma_chan);
        ad_dev->dma_chan = RT_NULL;
    }
    if (_ad7606_dma_event != RT_NULL)
    {
        rt_event_delete(_ad7606_dma_event);
        _ad7606_dma_event = RT_NULL;
    }
    if (ad_dev->dma_data != RT_NULL)
    {
        rt_free(ad_dev->dma_data);
        ad_dev->dma_data = RT_NULL;
    }
    ad_dev->sample_tid = RT_NULL;
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

    if (rt_ofw_prop_read_u32(
            np, "adi,oversampling_ratio", &ad_dev->oversampling) != RT_EOK)
    {
        ad_dev->oversampling = AD_OS_NO;
        LOG_W("Using default oversampling: none");
    }

    if (rt_ofw_prop_read_u32(np, "adi,range", &ad_dev->range) != RT_EOK)
    {
        ad_dev->range = AD_RANGE_5V;
        LOG_W("Using default range: ±5V");
    }

    if (rt_ofw_prop_read_string(np, "hwtimer-device", &ad_dev->hwtimer_name) !=
        RT_EOK)
    {
        ad_dev->hwtimer_name = "timer0";
        LOG_I("Using default hardware timer: timer0");
    }

    if (ad_dev->oversampling > AD_OS_X64)
    {
        LOG_W("Invalid oversampling ratio, using default");
        ad_dev->oversampling = AD_OS_NO;
    }

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

static rt_err_t ad7606_hwtimer_setup(
    struct ad7606_device *ad_dev, rt_bool_t enable)
{
    rt_err_t ret;

    if (enable)
    {
        if (!ad_dev->data_sem)
        {
            ad_dev->data_sem = rt_sem_create("ad7606_sem", 0, RT_IPC_FLAG_FIFO);
            if (!ad_dev->data_sem)
            {
                LOG_E("Create semaphore failed");
                return (-RT_ENOMEM);
            }
        }

        if (!ad_dev->sample_tid)
        {
            ad_dev->sample_run = RT_TRUE;
            ad_dev->data_ready = RT_FALSE;
            ad_dev->dma_enable = RT_FALSE;
            ad_dev->sample_tid = rt_thread_create(
                "ad7606_sample", ad7606_sample_entry, ad_dev, 2048, 15, 10);

            if (!ad_dev->sample_tid)
            {
                LOG_E("Create sample thread failed");
                rt_sem_delete(ad_dev->data_sem);
                ad_dev->data_sem = RT_NULL;
                return (-RT_ERROR);
            }
            rt_thread_startup(ad_dev->sample_tid);
        }

        ad_dev->hwtimer_dev = rt_device_find(ad_dev->hwtimer_name);
        if (!ad_dev->hwtimer_dev)
        {
            LOG_E("Hardware timer '%s' not found", ad_dev->hwtimer_name);
            goto error_cleanup;
        }

        ret = rt_device_open(ad_dev->hwtimer_dev, RT_DEVICE_OFLAG_RDWR);
        if (ret != RT_EOK)
        {
            LOG_E("Open hardware timer failed: %d", ret);
            goto error_cleanup;
        }

        ad_dev->hwtimer_dev->user_data = ad_dev;
        rt_device_set_rx_indicate(ad_dev->hwtimer_dev, ad7606_hwtimer_timeout);

        rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;
        ret = rt_device_control(
            ad_dev->hwtimer_dev, HWTIMER_CTRL_MODE_SET, &mode);
        if (ret != RT_EOK)
        {
            LOG_E("Set timer mode failed: %d", ret);
            goto error_close_timer;
        }

        rt_uint32_t freq = AD7606_SampleFreq[ad_dev->oversampling];
        rt_hwtimerval_t timeout = { 0, 1000000 / freq };
        ret =
            rt_device_write(ad_dev->hwtimer_dev, 0, &timeout, sizeof(timeout));
        if (ret != sizeof(timeout))
        {
            LOG_E("Set timer timeout failed");
            goto error_close_timer;
        }

        rt_pin_attach_irq(
            ad_dev->busy_pin, PIN_IRQ_MODE_FALLING, ad7606_busy_isr, ad_dev);
        rt_pin_irq_enable(ad_dev->busy_pin, PIN_IRQ_ENABLE);

        ad_dev->enabled = RT_TRUE;
        return RT_EOK;

error_close_timer:
        rt_device_close(ad_dev->hwtimer_dev);
        ad_dev->hwtimer_dev = RT_NULL;

error_cleanup:
        if (ad_dev->sample_tid)
        {
            ad_dev->sample_run = RT_FALSE;
            if (ad_dev->data_sem)
            {
                rt_sem_release(ad_dev->data_sem);
            }

            rt_tick_t start = rt_tick_get();
            while (ad_dev->sample_tid != RT_NULL)
            {
                if (rt_tick_get() - start > rt_tick_from_millisecond(100))
                {
                    LOG_W("sample thread exit timeout");
                    break;
                }
                rt_thread_mdelay(5);
            }

            if (ad_dev->sample_tid)
            {
                rt_thread_delete(ad_dev->sample_tid);
                ad_dev->sample_tid = RT_NULL;
            }
        }

        if (ad_dev->data_sem)
        {
            rt_sem_delete(ad_dev->data_sem);
            ad_dev->data_sem = RT_NULL;
        }

        return (-RT_ERROR);
    }
    else
    {
        ad_dev->enabled = RT_FALSE;
        ad_dev->data_ready = RT_FALSE;
        ad_dev->dma_enable = RT_FALSE;

        if (ad_dev->hwtimer_dev)
        {
            rt_device_control(ad_dev->hwtimer_dev, HWTIMER_CTRL_STOP, RT_NULL);
            rt_device_close(ad_dev->hwtimer_dev);
            ad_dev->hwtimer_dev = RT_NULL;
        }

        rt_pin_irq_enable(ad_dev->busy_pin, PIN_IRQ_DISABLE);
        rt_pin_detach_irq(ad_dev->busy_pin);

        if (ad_dev->sample_tid)
        {
            ad_dev->sample_run = RT_FALSE;
            if (ad_dev->data_sem)
            {
                rt_sem_release(ad_dev->data_sem);
            }

            rt_tick_t start = rt_tick_get();
            while (ad_dev->sample_tid != RT_NULL)
            {
                if (rt_tick_get() - start > rt_tick_from_millisecond(100))
                {
                    LOG_W("sample thread exit timeout, force delete");
                    rt_thread_delete(ad_dev->sample_tid);
                    ad_dev->sample_tid = RT_NULL;
                    break;
                }
                rt_thread_mdelay(5);
            }
        }

        if (ad_dev->data_sem)
        {
            rt_sem_delete(ad_dev->data_sem);
            ad_dev->data_sem = RT_NULL;
        }

        return RT_EOK;
    }
}

static rt_err_t ad7606_enabled(
    struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    struct ad7606_device *ad_dev;
    rt_err_t ret;

    ad_dev = rt_container_of(device, struct ad7606_device, adc_dev);

    if (enabled)
    {
        ad7606_gpio_init(ad_dev);
        ad7606_hard_reset(ad_dev);
        ad7606_set_os(ad_dev, ad_dev->oversampling);
        ad7606_set_range(ad_dev, ad_dev->range);

        ret = rt_adc_enable(ad_dev->fb_dev, channel);
        if (ret != RT_EOK)
        {
            LOG_E("Enable FlexBus ADC failed: %d", ret);
            return ret;
        }

        return ad7606_hwtimer_setup(ad_dev, RT_TRUE);
    }
    else
    {
        ret = ad7606_hwtimer_setup(ad_dev, RT_FALSE);
        rt_adc_disable(ad_dev->fb_dev, channel);
        return ret;
    }
}

static rt_err_t ad7606_read_raw(
    struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    struct ad7606_device *ad_dev;
    ad_dev = rt_container_of(device, struct ad7606_device, adc_dev);

    if ((channel < 1) || (channel > AD7606_MAX_CHANNELS))
    {
        LOG_D("Invalid channel: %d", channel);
        return (-RT_EINVAL);
    }

    if (!ad_dev->enabled)
    {
        LOG_D("AD7606 not enabled");
    }

    rt_tick_t start = rt_tick_get();
    while (!ad_dev->data_ready)
    {
        if (rt_tick_get() - start > rt_tick_from_millisecond(100))
        {
            LOG_D("Data not ready timeout");
            return (-RT_ETIMEOUT);
        }
        rt_thread_mdelay(1);
    }

    *value = (rt_uint32_t)ad_dev->data[channel - 1];

    return RT_EOK;
}

static rt_err_t ad7606_control(struct rt_adc_device *device, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;
    struct ad7606_device *ad_dev = rt_container_of(device, struct ad7606_device, adc_dev);
    struct rt_dma_slave_config dma_cfg = {0};
    rt_adc_dma_cfg_t adc_dma_cfg = (rt_adc_dma_cfg_t)args;

    if (ad_dev->enabled != RT_TRUE)
    {
        LOG_W("AD7606 not enabled");
        return -RT_ERROR;
    }

    switch (cmd)
    {
        case RT_ADC_CMD_DMA_START:
        {
            if (ad_dev->dma_enable == RT_TRUE)
            {
                LOG_I("DMA already enabled, skipping initialization");
                break;
            }

            if (ad_dev->dma_chan == RT_NULL)
            {
                ad_dev->dma_chan = rt_dma_chan_request(ad_dev->dev, RT_NULL);
                if (ad_dev->dma_chan == RT_NULL)
                {
                    LOG_E("DMA channel request failed");
                    ret = -RT_ERROR;
                    break;
                }
                ad_dev->dma_chan->callback = ad7606_dma_isr;
            }

            if (_ad7606_dma_event == RT_NULL)
            {
                _ad7606_dma_event = rt_event_create("ad7606_event", RT_IPC_FLAG_FIFO);
                if (_ad7606_dma_event == RT_NULL)
                {
                    LOG_E("DMA event create failed");
                    ret = -RT_ERROR;
                    goto cleanup_dma_chan;
                }
            }

            if (ad_dev->dma_data == RT_NULL)
            {
                ad_dev->dma_data = (rt_uint16_t *)rt_calloc(1, adc_dma_cfg->buf_len);
                if (ad_dev->dma_data == RT_NULL)
                {
                    LOG_E("DMA memory alloc failed");
                    ret = -RT_ENOMEM;
                    goto cleanup_event;
                }
            }

            ad_dev->dma_cfg = *adc_dma_cfg;

            dma_cfg.direction = RT_DMA_MEM_TO_MEM;
            dma_cfg.src_addr = (rt_ubase_t)ad_dev->dma_data;
            dma_cfg.dst_addr = (rt_ubase_t)adc_dma_cfg->dst_addr;
            ret = rt_dma_chan_config(ad_dev->dma_chan, &dma_cfg);
            if (ret != RT_EOK)
            {
                LOG_E("DMA config failed");
                ret = -RT_ERROR;
                goto cleanup_dma_data;
            }

            ad_dev->dma_enable = RT_TRUE;
            break;

cleanup_dma_data:
            rt_free(ad_dev->dma_data);
            ad_dev->dma_data = RT_NULL;
cleanup_event:
            rt_event_delete(_ad7606_dma_event);
            _ad7606_dma_event = RT_NULL;
cleanup_dma_chan:
            rt_dma_chan_release(ad_dev->dma_chan);
            ad_dev->dma_chan = RT_NULL;
            ad_dev->dma_enable = RT_FALSE;
            break;
        }
        case RT_ADC_CMD_DMA_STOP:
        {
            ad_dev->dma_enable = RT_FALSE;
            if (ad_dev->dma_chan != RT_NULL)
            {
                rt_dma_chan_stop(ad_dev->dma_chan);
            }
            break;
        }
        default:
        {
            LOG_E("Invalid command");
            break;
        }
    }

    return ret;
}

static const struct rt_adc_ops ad7606_adc_ops = {
    .enabled = ad7606_enabled,
    .convert = ad7606_read_raw,
    .control = ad7606_control,
};

static rt_err_t ad7606_init(struct rt_device *dev)
{
    const char *dev_name;
    struct ad7606_device *ad_dev;
    struct rt_ofw_node *np = dev->ofw_node;
    rt_err_t ret;

    ad_dev = rt_calloc(1, sizeof(struct ad7606_device));
    if (!ad_dev)
    {
        LOG_E("Memory allocation failed");
        return (-RT_ENOMEM);
    }

    ad_dev->dev = dev;
    dev->user_data = ad_dev;

    ad_dev->fb_dev = (rt_adc_device_t)rt_device_find("flexbus_adc0");
    if (!ad_dev->fb_dev)
    {
        LOG_E("FlexBus ADC device not found");
        rt_free(ad_dev);
        return (-RT_ERROR);
    }

    ret = ad7606_request_gpios(np, ad_dev);
    if (ret != RT_EOK)
    {
        LOG_E("GPIO configuration failed");
        rt_free(ad_dev);
        return ret;
    }

    rt_dm_dev_set_name_auto(dev, "adc");
    dev_name = rt_dm_dev_get_name(dev);

    ad_dev->adc_dev.ops = &ad7606_adc_ops;
    ret = rt_hw_adc_register(
        &ad_dev->adc_dev, dev_name, &ad7606_adc_ops, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("ADC device register failed: %d", ret);
        rt_free(ad_dev);
        return ret;
    }

    LOG_I("AD7606 device registered: %s", dev_name);
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
