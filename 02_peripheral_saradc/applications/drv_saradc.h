/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DRV_SARDAC_H__
#define __DRV_SARDAC_H__

#include <rtthread.h>
#include <rtdevice.h>

struct rk_saradc_regs
{
    rt_uint32_t conv_con;
    rt_uint32_t t_pd_soc;
    rt_uint32_t t_as_soc;
    rt_uint32_t t_das_soc;
    rt_uint32_t t_sel_soc;
    rt_uint32_t high_comp0;
    rt_uint32_t high_comp1;
    rt_uint32_t high_comp2;
    rt_uint32_t high_comp3;
    rt_uint32_t reserved0024[12];
    rt_uint32_t low_comp0;
    rt_uint32_t low_comp1;
    rt_uint32_t low_comp2;
    rt_uint32_t low_comp3;
    rt_uint32_t reserved0064[12];
    rt_uint32_t debounce;
    rt_uint32_t ht_int_en;
    rt_uint32_t lt_int_en;
    rt_uint32_t reserved00A0[24];
    rt_uint32_t mt_int_en;
    rt_uint32_t end_int_en;
    rt_uint32_t st_con;
    rt_uint32_t status;
    rt_uint32_t end_int_st;
    rt_uint32_t ht_int_st;
    rt_uint32_t lt_int_st;
    rt_uint32_t mt_int_st;
    rt_uint32_t data0;
    rt_uint32_t data1;
    rt_uint32_t data2;
    rt_uint32_t data3;
    rt_uint32_t reserved0130[12];
    rt_uint32_t auto_ch_en;
};

struct rk_saradc_data
{
    rt_uint32_t num_bits;
    rt_uint8_t num_channels;
    rt_uint32_t mverf;
};

typedef struct
{
    volatile struct rk_saradc_regs *regs;
    const struct rk_saradc_data *data;
    rt_int8_t active_channel;
} rk_saradc;

rt_err_t saradc_read(rk_saradc *saradc, rt_uint8_t channel, rt_uint16_t *data);
rt_uint32_t saradc_to_voltage(rk_saradc *saradca, rt_uint8_t channel);
void saradc_stop(rk_saradc *saradc);
rt_err_t saradc_start_channel(rk_saradc *saradc, rt_uint8_t channel);
rk_saradc *saradc_init(void);

#endif // __DRV_SARDAC_H__