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
    AD_OS_NO = 0,
    AD_OS_X2 = 1,
    AD_OS_X4 = 2,
    AD_OS_X8 = 3,
    AD_OS_X16 = 4,
    AD_OS_X32 = 5,
    AD_OS_X64 = 6,
} ad7606_os_t;

struct ad7606_device
{
    struct rt_adc_device adc_dev;
    rt_adc_device_t fb_dev;
    struct rt_device *dev;

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

    rt_int16_t data[8];
};

#endif /* __DRV_AD7606_H__ */
