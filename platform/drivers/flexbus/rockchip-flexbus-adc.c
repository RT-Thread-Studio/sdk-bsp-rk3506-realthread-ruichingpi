/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rockchip_flexbus.h>

// #define DRV_DEBUG
#define DBG_TAG "flexbus.adc"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_WARNING
#endif /* DRV_DEBUG */
#include <rtdbg.h>

enum flexbus_adc_result
{
    ADC_DONE = 0,
    ADC_ERR,
};

struct rockchip_flexbus_adc
{
    struct rt_device *dev;
    struct rockchip_flexbus *rkfb;
    struct rt_clk *ref_clk;
    rt_uint32_t dfs;
    rt_bool_t slave_mode;
    rt_bool_t free_sclk;
    rt_bool_t auto_pad;
    rt_bool_t cpol;
    rt_bool_t cpha;
    struct rt_mutex lock;
    struct rt_completion completion;
    enum flexbus_adc_result result;
    rt_uint32_t dst_buf_len;
    void *dst_buf;
    struct rt_adc_device adc_device;
};
typedef struct rockchip_flexbus_adc rockchip_flexbus_adc_t;

static int rockchip_flexbus_adc_read_block(
    struct rockchip_flexbus_adc *rkfb_adc,
    void *dst_buf,
    rt_uint32_t dst_len,
    rt_uint32_t num_of_dfs)
{
    struct rockchip_flexbus *rkfb = rkfb_adc->rkfb;
    int ret = 0;

    if (num_of_dfs > 0x08000000)
    {
        LOG_E("num_of_dfs is too large");
        return (-RT_EINVAL);
    }

    rt_mutex_take(&rkfb_adc->lock, RT_WAITING_FOREVER);
    rt_completion_init(&rkfb_adc->completion);

    rockchip_flexbus_writel(rkfb, FLEXBUS_RX_NUM, num_of_dfs);
    rockchip_flexbus_writel(
        rkfb, FLEXBUS_DMA_DST_ADDR0, (rt_uint32_t)dst_buf >> 2);
    rockchip_flexbus_writel(rkfb, FLEXBUS_DMA_DST_LEN0, dst_len);

    rockchip_flexbus_writel(rkfb, FLEXBUS_ENR, FLEXBUS_RX_ENR);

    if (rt_completion_wait(&rkfb_adc->completion, FLEXBUS_ADC_TIMEOUT) !=
        RT_EOK)
    {
        ret = (-RT_ETIMEOUT);
        LOG_E("flexbus adc read timeout");
        goto end;
    }

    if (rkfb_adc->result != ADC_DONE)
    {
        ret = -1;
        LOG_E("adc read error: %d", rkfb_adc->result);
    }

    rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, rkfb_adc->dst_buf,
        DIV_ROUND_UP(8 * rkfb_adc->dfs, 8));

end:
    rockchip_flexbus_writel(rkfb, FLEXBUS_ENR, FLEXBUS_RX_DIS);
    rt_mutex_release(&rkfb_adc->lock);

    return ret;
}

static rt_err_t rockchip_flexbus_adc_read_raw(
    struct rt_adc_device *device, rt_int8_t channel, rt_uint32_t *value)
{
    struct rockchip_flexbus_adc *rkfb_adc =
        rt_container_of(device, struct rockchip_flexbus_adc, adc_device);
    rt_uint16_t *pBuf;
    rt_uint32_t val_mask;
    int ret;

    ret = rockchip_flexbus_adc_read_block(
        rkfb_adc, rkfb_adc->dst_buf, rkfb_adc->dst_buf_len, 8);
    if (ret)
    {
        return ret;
    }

    switch (rkfb_adc->dfs)
    {
    case 4: val_mask = 0xf; break;
    case 8: val_mask = 0xff; break;
    case 16:
    default: val_mask = 0xffff; break;
    }

    pBuf = (rt_uint16_t *)rkfb_adc->dst_buf;
    *value = pBuf[0] & val_mask;

    rockchip_flexbus_writel(rkfb_adc->rkfb, FLEXBUS_ENR, FLEXBUS_RX_DIS);
    rt_mutex_release(&rkfb_adc->lock);

    return RT_EOK;
}

static void rockchip_flexbus_adc_isr(
    struct rockchip_flexbus *rkfb, rt_uint32_t isr)
{
    struct rockchip_flexbus_adc *rkfb_adc = rkfb->fb1_data;

    if (rkfb->opmode1 != ROCKCHIP_FLEXBUS1_OPMODE_ADC)
    {
        return;
    }

    if (isr & FLEXBUS_RX_DONE_ISR)
    {
        rkfb_adc->result = ADC_DONE;
        rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_RX_DONE_ISR);
    }

    if (isr & FLEXBUS_DMA_DST1_ISR)
    {
        rkfb_adc->result = ADC_DONE;
        rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_DMA_DST1_ISR);
    }

    if (isr & FLEXBUS_DMA_DST0_ISR)
    {
        rkfb_adc->result = ADC_DONE;
        rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_DMA_DST0_ISR);
    }

    if (isr & FLEXBUS_ADC_ERR_ISR)
    {
        rkfb_adc->result = ADC_ERR;

        if (isr & FLEXBUS_DMA_TIMEOUT_ISR)
        {
            LOG_E("dma timeout!");
            rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_DMA_TIMEOUT_ISR);
        }

        if (isr & FLEXBUS_DMA_ERR_ISR)
        {
            LOG_E("dma err!");
            rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_DMA_ERR_ISR);
        }

        if (isr & FLEXBUS_RX_UDF_ISR)
        {
            LOG_E("rx underflow!");
            rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_RX_UDF_ISR);
        }

        if (isr & FLEXBUS_RX_OVF_ISR)
        {
            LOG_E("rx overflow!");
            rockchip_flexbus_writel(rkfb, FLEXBUS_ICR, FLEXBUS_RX_OVF_ISR);
        }
    }

    rt_completion_done(&rkfb_adc->completion);
}

static int flexbus_adc_init(struct rockchip_flexbus_adc *rkfb_adc)
{
    struct rockchip_flexbus *rkfb = rkfb_adc->rkfb;
    rt_uint32_t val = 0;

    rockchip_flexbus_writel(rkfb, FLEXBUS_FREE_SCLK, FLEXBUS_RX_FREE_MODE);
    rockchip_flexbus_writel(rkfb, FLEXBUS_SLAVE_MODE, val);

    switch (rkfb_adc->dfs)
    {
    case 4: val = rkfb->dfs_reg->dfs_4bit; break;
    case 8: val = rkfb->dfs_reg->dfs_8bit; break;
    case 16: val = rkfb->dfs_reg->dfs_16bit; break;
    default: return (-RT_EINVAL);
    }
    if (rkfb_adc->auto_pad)
    {
        val |= FLEXBUS_AUTOPAD;
    }

    if (rkfb_adc->cpol)
    {
        val |= FLEXBUS_CPOL;
    }

    if (rkfb_adc->cpha)
    {
        val |= FLEXBUS_CPHA;
    }

    rockchip_flexbus_writel(rkfb, FLEXBUS_RX_CTL, val);
    rockchip_flexbus_setbits(rkfb, FLEXBUS_IMR, 0x3fff);

    rk3506_flexbus_grf_config(
        rkfb, rkfb_adc->slave_mode, rkfb_adc->cpol, rkfb_adc->cpha);

    return 0;
}

static int rockchip_flexbus_adc_parse_dt(struct rockchip_flexbus_adc *rkfb_adc)
{
    rkfb_adc->slave_mode =
        !rt_dm_dev_prop_read_bool(rkfb_adc->dev, "rockchip,slave-mode");
    rkfb_adc->auto_pad =
        rt_dm_dev_prop_read_bool(rkfb_adc->dev, "rockchip,auto-pad");
    rkfb_adc->cpol = rt_dm_dev_prop_read_bool(rkfb_adc->dev, "rockchip,cpol");
    rkfb_adc->cpha = rt_dm_dev_prop_read_bool(rkfb_adc->dev, "rockchip,cpha");
    rkfb_adc->free_sclk =
        rt_dm_dev_prop_read_bool(rkfb_adc->dev, "rockchip,free-sclk");

    if (rt_dm_dev_prop_read_u32(rkfb_adc->dev, "rockchip,dfs", &rkfb_adc->dfs))
    {
        LOG_E("failed to get dfs");
        return (-RT_EINVAL);
    }
    if (rkfb_adc->dfs != 4 && rkfb_adc->dfs != 8 && rkfb_adc->dfs != 16)
    {
        LOG_E("invalid dfs");
        return (-RT_EINVAL);
    }

    return 0;
}

rt_err_t rockchip_flexbus_adc_enabled(
    struct rt_adc_device *device, rt_int8_t channel, rt_bool_t enabled)
{
    return RT_EOK;
}

static const struct rt_adc_ops rockchip_flexbus_adc_ops = {
    .enabled = rockchip_flexbus_adc_enabled,
    .convert = rockchip_flexbus_adc_read_raw,
};

static int rockchip_flexbus_adc_init(struct rt_device *dev)
{
    rt_err_t ret;
    const char *dev_name;

    struct rt_ofw_node *np = dev->ofw_node;
    struct rt_ofw_node *npp = rt_ofw_get_parent(np);

    struct rockchip_flexbus *rkfb = rt_ofw_data(npp);
    struct rockchip_flexbus_adc *rkfb_adc = rt_calloc(1, sizeof(*rkfb_adc));

    if (rkfb->opmode1 != ROCKCHIP_FLEXBUS1_OPMODE_ADC)
    {
        LOG_E("flexbus1 opmode mismatch opmode1[%d]", rkfb->opmode1);
        return (-RT_EIO);
    }

    rkfb_adc->dev = dev;
    rkfb_adc->rkfb = rkfb;
    rkfb_adc->adc_device.ops = &rockchip_flexbus_adc_ops;

    ret = rockchip_flexbus_adc_parse_dt(rkfb_adc);
    if (ret)
    {
        goto err_fb1;
    }

    if (rkfb_adc->slave_mode)
    {
        rkfb_adc->ref_clk = rt_clk_get_by_name(dev, "ref_clk");
        if (rkfb_adc->ref_clk)
        {
            LOG_I(
                "failed to get ref_clk in slave-mode. Please make sure the ADC device has clock source");
        }
        else
        {
            rt_clk_enable(rkfb_adc->ref_clk);
        }
    }

    rt_mutex_init(&rkfb_adc->lock, "rkfb_adc", RT_IPC_FLAG_FIFO);
    rt_completion_init(&rkfb_adc->completion);

    ret = flexbus_adc_init(rkfb_adc);
    if (ret)
    {
        LOG_E("failed to init flexbus adc");
        goto err_mutex_destroy;
    }

    rt_dm_dev_set_name_auto(dev, "flexbus_adc");
    dev_name = rt_dm_dev_get_name(dev);

    rkfb_adc->dst_buf_len =
        RT_ALIGN(FLEXBUS_ADC_REPEAT * rkfb_adc->dfs / 8, 0x40);
    if (rkfb_adc->dst_buf_len < 0x40 || rkfb_adc->dst_buf_len > 0x0fffffc0)
    {
        LOG_E("buf_len should >= 0x40 and <= 0x0fffffc0");
        ret = (-RT_ERROR);
        goto err_mutex_destroy;
    }

    rkfb_adc->dst_buf = rt_malloc_align(rkfb_adc->dst_buf_len, 0x40);
    if (!rkfb_adc->dst_buf)
    {
        ret = (-RT_ENOMEM);
        goto err_mutex_destroy;
    }

    rt_memset(rkfb_adc->dst_buf, 0, rkfb_adc->dst_buf_len);
    rt_hw_cpu_dcache_ops(
        RT_HW_CACHE_FLUSH, rkfb_adc->dst_buf, rkfb_adc->dst_buf_len);

    rockchip_flexbus_set_fb1(rkfb, rkfb_adc, rockchip_flexbus_adc_isr);

    ret = rt_hw_adc_register(
        &rkfb_adc->adc_device, dev_name, &rockchip_flexbus_adc_ops, RT_NULL);
    if (ret != RT_EOK)
    {
        LOG_E("%s register fail", dev_name);
        ret = (-RT_ERROR);
        goto err_fb1;
    }

    return ret;

err_mutex_destroy:
    rt_mutex_detach(&rkfb_adc->lock);
err_fb1:
    rockchip_flexbus_set_fb1(rkfb, RT_NULL, RT_NULL);

    return ret;
}

static rt_err_t rockchip_flexbus_adc_probe(struct rt_platform_device *pdev)
{
    struct rt_ofw_node *np, *device_np;

    if (rockchip_flexbus_adc_init(&pdev->parent))
    {
        LOG_E("rk3506 flexbus adc init failed");
        return (-RT_ERROR);
    }

    np = pdev->parent.ofw_node;
    rt_ofw_foreach_available_child_node(np, device_np)
    {
        if (!rt_ofw_prop_read_bool(device_np, "compatible"))
        {
            continue;
        }

        rt_platform_ofw_device_probe_child(device_np);
    }

    LOG_I("rockchip_flexbus_adc_probe");

    return RT_EOK;
}

static const struct rt_ofw_node_id rk3506_flexbus_ofw_ids[] = {
    { .compatible = "rockchip,flexbus-adc" }, { /* sentinel */ }
};

static struct rt_platform_driver rk3506_flexbus_driver = {
    .name = "rockchip,flexbus-adc",
    .ids = rk3506_flexbus_ofw_ids,
    .probe = rockchip_flexbus_adc_probe,
};

int rk3506_flexbus_drv_register(void)
{
    return rt_platform_driver_register(&rk3506_flexbus_driver);
}
INIT_DEVICE_EXPORT(rk3506_flexbus_drv_register);
