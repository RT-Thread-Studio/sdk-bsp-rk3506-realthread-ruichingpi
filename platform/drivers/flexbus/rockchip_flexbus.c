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
#define DBG_TAG "drv.flexbus"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_WARNING
#endif /* DRV_DEBUG */
#include <rtdbg.h>

rt_uint32_t rockchip_flexbus_readl(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg)
{
    return readl(rkfb->base + reg);
}

void rockchip_flexbus_writel(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t val)
{
    writel(val, rkfb->base + reg);
}

void rockchip_flexbus_clrbits(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t clr_val)
{
    rt_uint32_t reg_val;

    reg_val = rockchip_flexbus_readl(rkfb, reg);
    reg_val &= ~(clr_val);
    rockchip_flexbus_writel(rkfb, reg, reg_val);
}

void rockchip_flexbus_setbits(
    struct rockchip_flexbus *rkfb, rt_uint32_t reg, rt_uint32_t set_val)
{
    rt_uint32_t reg_val;

    reg_val = rockchip_flexbus_readl(rkfb, reg);
    reg_val |= set_val;
    rockchip_flexbus_writel(rkfb, reg, reg_val);
}

void rockchip_flexbus_clrsetbits(struct rockchip_flexbus *rkfb,
    rt_uint32_t reg,
    rt_uint32_t clr_val,
    rt_uint32_t set_val)
{
    rt_uint32_t reg_val;

    reg_val = rockchip_flexbus_readl(rkfb, reg);
    reg_val &= ~(clr_val);
    reg_val |= set_val;
    rockchip_flexbus_writel(rkfb, reg, reg_val);
}

static struct rockchip_flexbus_dfs_reg rockchip_flexbus_dfs_reg_v0 = {
    .dfs_2bit = 0x0,
    .dfs_4bit = 0x1,
    .dfs_8bit = 0x2,
    .dfs_16bit = 0x3,
    .dfs_mask = 0x3,
};

static struct rockchip_flexbus_dfs_reg rockchip_flexbus_dfs_reg_v1 = {
    .dfs_1bit = (0x0 << 29),
    .dfs_2bit = (0x1 << 29),
    .dfs_4bit = (0x2 << 29),
    .dfs_8bit = (0x3 << 29),
    .dfs_16bit = (0x4 << 29),
    .dfs_mask = (0x7 << 29),
};

static void rk3506_flexbus_init_config(struct rockchip_flexbus *rkfb)
{
    writel(RT_BIT(4 + 16), rkfb->regmap_grf + RK3506_GRF_SOC_CON1);
}

void rk3506_flexbus_grf_config(
    struct rockchip_flexbus *rkfb, bool slave_mode, bool cpol, bool cpha)
{
    rt_uint32_t val = 0x3 << 16;

    if (slave_mode)
    {
        if ((!cpol && cpha) || (cpol && !cpha))
        {
            val |= RT_BIT(0);
        }
    }
    else
    {
        val |= RT_BIT(1);
    }

    writel(val, rkfb->regmap_grf + RK3506_GRF_SOC_CON1);
}

static void rk3506_rkfb_handle(int irq, void *parm)
{
    struct rockchip_flexbus *rkfb = parm;
    rt_uint32_t isr = 0;

    isr = rockchip_flexbus_readl(rkfb, FLEXBUS_ISR);

    if (rkfb->opmode0 != ROCKCHIP_FLEXBUS0_OPMODE_NULL && rkfb->fb0_isr)
    {
        rkfb->fb0_isr(rkfb, isr);
    }

    if (rkfb->opmode1 != ROCKCHIP_FLEXBUS1_OPMODE_NULL && rkfb->fb1_isr)
    {
        rkfb->fb1_isr(rkfb, isr);
    }
}

static int rk3506_flexbus_init(struct rt_device *dev)
{
    rt_err_t err = RT_EOK;
    const char *dev_name;
    struct rt_ofw_node *np = dev->ofw_node;
    struct rockchip_flexbus *rkfb = rt_calloc(1, sizeof(*rkfb));

    err = rt_dm_dev_prop_read_u32(
        dev, "rockchip,flexbus0-opmode", &rkfb->opmode0);
    if (err)
    {
        rkfb->opmode0 = ROCKCHIP_FLEXBUS0_OPMODE_NULL;
    }

    if (rkfb->opmode0 < ROCKCHIP_FLEXBUS0_OPMODE_NULL ||
        rkfb->opmode0 > ROCKCHIP_FLEXBUS0_OPMODE_SPI)
    {
        return (-RT_EINVAL);
    }

    err = rt_dm_dev_prop_read_u32(
        dev, "rockchip,flexbus1-opmode", &rkfb->opmode1);
    if (err)
    {
        rkfb->opmode1 = ROCKCHIP_FLEXBUS1_OPMODE_NULL;
    }

    if (rkfb->opmode1 < ROCKCHIP_FLEXBUS1_OPMODE_NULL ||
        rkfb->opmode1 > ROCKCHIP_FLEXBUS1_OPMODE_CIF)
    {
        return (-RT_EINVAL);
    }

    rkfb->irq = rt_dm_dev_get_irq(dev, 0);
    rkfb->base = rt_ofw_iomap(np, 0);
    if (!rkfb->base)
    {
        err = (-RT_EIO);
        goto _fail;
    }

    rkfb->regmap_grf = rt_syscon_find_by_ofw_phandle(np, "rockchip,grf");
    if (!rkfb->regmap_grf)
    {
        LOG_E("failed to get rockchip,grf node[0x%x]", rkfb->regmap_grf);
        err = (-RT_EIO);
        goto _fail;
    }

    if (!(rkfb->tx_clk_flexbus = rt_clk_get_by_name(dev, "tx_clk_flexbus")))
    {
        LOG_E("failed to get tx_clk_flexbus node[0x%x]", rkfb->tx_clk_flexbus);

        err = (-RT_EIO);
        goto _fail;
    }

    if (!(rkfb->rx_clk_flexbus = rt_clk_get_by_name(dev, "rx_clk_flexbus")))
    {
        LOG_E("failed to get rx_clk_flexbus node[0x%x]", rkfb->rx_clk_flexbus);
        err = (-RT_EIO);
        goto _fail;
    }

    if (!(rkfb->aclk_flexbus = rt_clk_get_by_name(dev, "aclk_flexbus")))
    {
        LOG_E("failed to get aclk_flexbus node[0x%x]", rkfb->aclk_flexbus);
        err = (-RT_EIO);
        goto _fail;
    }

    if (!(rkfb->hclk_flexbus = rt_clk_get_by_name(dev, "hclk_flexbus")))
    {
        LOG_E("failed to get hclk_flexbus node[0x%x]", rkfb->hclk_flexbus);
        err = (-RT_EIO);
        goto _fail;
    }
    rt_clk_enable(rkfb->tx_clk_flexbus);
    rt_clk_enable(rkfb->rx_clk_flexbus);
    rt_clk_enable(rkfb->aclk_flexbus);
    rt_clk_enable(rkfb->hclk_flexbus);

    rk3506_flexbus_init_config(rkfb);

    rt_dm_dev_set_name_auto(dev, "flexbus");
    dev_name = rt_dm_dev_get_name(dev);

    if ((rkfb->opmode0 != ROCKCHIP_FLEXBUS0_OPMODE_NULL) &&
        (rkfb->opmode1 != ROCKCHIP_FLEXBUS1_OPMODE_NULL))
    {
        rockchip_flexbus_writel(rkfb, FLEXBUS_COM_CTL, FLEXBUS_TX_AND_RX);
    }
    else if (rkfb->opmode0 != ROCKCHIP_FLEXBUS0_OPMODE_NULL)
    {
        rockchip_flexbus_writel(rkfb, FLEXBUS_COM_CTL, FLEXBUS_TX_ONLY);
    }
    else
    {
        rockchip_flexbus_writel(rkfb, FLEXBUS_COM_CTL, FLEXBUS_RX_ONLY);
    }

    switch (rockchip_flexbus_readl(rkfb, FLEXBUS_REVISION) >> 24 & 0xff)
    {
    case 0x0: rkfb->dfs_reg = &rockchip_flexbus_dfs_reg_v0; break;
    case 0x1: rkfb->dfs_reg = &rockchip_flexbus_dfs_reg_v1; break;
    default:
        LOG_E("failed to get large version");
        err = (-RT_EIO);
        goto _fail;
    }

    rt_ofw_data(np) = rkfb;

    rt_hw_interrupt_install(rkfb->irq, rk3506_rkfb_handle, rkfb, dev_name);
    rt_hw_interrupt_umask(rkfb->irq);

    return RT_EOK;

_fail:

    LOG_E("rk3506 flexbus probe failed");

    rt_free(rkfb);

    return err;
}

static rt_err_t rk3506_flexbus_probe(struct rt_platform_device *pdev)
{
    struct rt_ofw_node *np, *device_np;

    if (rk3506_flexbus_init(&pdev->parent))
    {
        LOG_E("rk3506 flexbus init failed");
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

    LOG_I("rk3506_flexbus_probe");

    return RT_EOK;
}

static const struct rt_ofw_node_id rk3506_flexbus_ofw_ids[] = {
    { .compatible = "rockchip,rk3506-flexbus" }, { /* sentinel */ }
};

static struct rt_platform_driver rk3506_flexbus_driver = {
    .name = "rockchip,rk3506-flexbus",
    .ids = rk3506_flexbus_ofw_ids,
    .probe = rk3506_flexbus_probe,
};

int rk_flexbus_drv_register(void)
{
    return rt_platform_driver_register(&rk3506_flexbus_driver);
}
INIT_PREV_EXPORT(rk_flexbus_drv_register);
