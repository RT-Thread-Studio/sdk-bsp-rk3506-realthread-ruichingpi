/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "drv_saradc.h"

#define SARADC_BASE_ADDR    0xFF4E8000
#define BIT(X)              (1 << (X))
#define SARADC2_EN_END_INT  BIT(0)
#define SARADC2_START       BIT(4)
#define SARADC2_SINGLE_MODE BIT(5)

#define SARADC_TIMEOUT      (50)

rt_err_t saradc_read(rk_saradc *saradc, rt_uint8_t channel, rt_uint16_t *data)
{
    if (saradc == RT_NULL)
    {
        rt_kprintf("Saradc is NULL\n");
        return (-RT_EINVAL);
    }

    if (channel != saradc->active_channel)
    {
        rt_kprintf("Channel is out of range\n");
        return (-RT_EINVAL);
    }

    rt_tick_t tick_timeout = rt_tick_get() + SARADC_TIMEOUT;
    while (!(readl(&saradc->regs->end_int_st) & SARADC2_EN_END_INT))
    {
        if (rt_tick_get() > tick_timeout)
        {
            rt_kprintf("Wait for end conversion interrupt status timeout!\n");
            return (-RT_ETIMEOUT);
        }
    }

    /* Clear irq. */
    writel(0x1, &saradc->regs->end_int_st);

    *data = readl(&saradc->regs->data0 + channel);
    *data &= ((1 << saradc->data->num_bits) - 1);

    return RT_EOK;
}

rt_uint32_t saradc_to_voltage(rk_saradc *saradca, rt_uint8_t channel)
{
    rt_uint16_t data = 0;
    saradc_read(saradca, channel, &data);

    return ((data + 1) * saradca->data->mverf) / (1 << saradca->data->num_bits);
}

void saradc_stop(rk_saradc *saradc)
{
    if (saradc == RT_NULL)
    {
        rt_kprintf("Saradc is NULL\n");
        return;
    }

    saradc->active_channel = -1;
}

rt_err_t saradc_start_channel(rk_saradc *saradc, rt_uint8_t channel)
{
    rt_uint32_t reg = 0;

    writel(0xffff0000, &saradc->regs->conv_con);

    if (saradc == RT_NULL)
    {
        rt_kprintf("Saradc is NULL\n");
        return (-RT_ERROR);
    }

    if (channel > 3 || channel < 0)
    {
        rt_kprintf("Channel is out of range\n");
        return (-RT_ERROR);
    }

    writel(0x20, &saradc->regs->t_pd_soc);
    writel(0xc, &saradc->regs->t_das_soc);

    reg = SARADC2_EN_END_INT << 16 | SARADC2_EN_END_INT;
    writel(reg, &saradc->regs->end_int_en);

    reg = SARADC2_START | SARADC2_SINGLE_MODE | channel;
    writel(reg << 16 | reg, &saradc->regs->conv_con);

    rt_thread_mdelay(1);

    saradc->active_channel = channel;

    return RT_EOK;
}

rk_saradc *saradc_init(void)
{
    static struct rk_saradc_data saradc_data = {
        .num_bits = 10,
        .num_channels = 4,
        .mverf = 1800,
    };

    static rk_saradc saradc = {
        .regs = (volatile struct rk_saradc_regs *)(void *)SARADC_BASE_ADDR,
        .data = &saradc_data,
        .active_channel = -1,
    };

    return &saradc;
}
