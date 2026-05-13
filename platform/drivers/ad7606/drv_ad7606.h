/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DRV_AD7606_H__
#define __DRV_AD7606_H__

#include <rtthread.h>
#include <rtdevice.h>

typedef enum
{
    AD_OS_NO = 0,  /* 无过采样，200KSPS */
    AD_OS_X2 = 1,  /* 2倍过采样，100KSPS */
    AD_OS_X4 = 2,  /* 4倍过采样，50KSPS */
    AD_OS_X8 = 3,  /* 8倍过采样，25KSPS */
    AD_OS_X16 = 4, /* 16倍过采样，12.5KSPS */
    AD_OS_X32 = 5, /* 32倍过采样，6.25KSPS */
    AD_OS_X64 = 6, /* 64倍过采样，3.125KSPS */
} ad7606_os_t;


typedef enum
{
    AD_RANGE_5V = 0,  /* ±5V量程 */
    AD_RANGE_10V = 1, /* ±10V量程 */
} ad7606_range_t;

#define AD7606_MAX_CHANNELS          (8)
#define AD7606_CHANNEL(ch)           (1 << (ch - 1))

struct ad7606_device
{
    struct rt_adc_device adc_dev;
    rt_adc_device_t fb_dev;
    struct rt_device *dev;
    struct rt_dma_chan *dma_chan;

    rt_uint32_t cs_pin;
    rt_uint32_t rd_pin;
    rt_uint32_t rst_pin;
    rt_uint32_t busy_pin;
    rt_uint32_t range_pin;
    rt_uint32_t cva_pin;
    rt_uint32_t cvb_pin;
    rt_uint32_t os0_pin;
    rt_uint32_t os1_pin;
    rt_uint32_t os2_pin;

    rt_uint32_t oversampling;
    rt_uint32_t range;

    const char *hwtimer_name;
    rt_device_t hwtimer_dev;

    rt_sem_t data_sem;
    rt_thread_t sample_tid;
    volatile rt_bool_t sample_run;
    volatile rt_bool_t enabled;
    volatile rt_bool_t data_ready;

    volatile rt_uint16_t data[AD7606_MAX_CHANNELS];

    struct rt_adc_dma_cfg dma_cfg;
    rt_bool_t dma_enable;
    rt_uint16_t *dma_data;
};

int ad7606_driver_register(void);

#endif /* __DRV_AD7606_H__ */
