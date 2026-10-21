/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __TSC2046_H__
#define __TSC2046_H__

#include <rtdevice.h>

#define TSC2046_CMD_X                0xd1
#define TSC2046_CMD_Y                0x91
#define TSC2046_CMD_Z1               0xb1
#define TSC2046_CMD_Z2               0xc1
#define TSC2046_CMD_POWER_DOWN       0x90

#define TSC2046_MAX_12BIT            0x0fff
#define TSC2046_DEFAULT_MAX_HZ       1000000
#define TSC2046_FILTER_SAMPLES       5
#define TSC2046_FILTER_MAX_SPREAD    120
#define TSC2046_TRANSFER_BYTES       3
#define TSC2046_CHANNEL_COUNT        4
#define TSC2046_MAX_SETTLE_SAMPLES   64

#define TSC2046_DEFAULT_X_MIN        200
#define TSC2046_DEFAULT_X_MAX        3900
#define TSC2046_DEFAULT_Y_MIN        200
#define TSC2046_DEFAULT_Y_MAX        3900
#define TSC2046_DEFAULT_X_SIZE       800
#define TSC2046_DEFAULT_Y_SIZE       480
#define TSC2046_DEFAULT_X_PLATE_OHMS 400
#define TSC2046_DEFAULT_PRESSURE_MAX 0xffff

struct tsc2046_data
{
    struct rt_touch_device touch;
    struct rt_spi_device *spi;
    rt_int32_t irq_pin;
    rt_int32_t raw_x_min;
    rt_int32_t raw_x_max;
    rt_int32_t raw_y_min;
    rt_int32_t raw_y_max;
    rt_int32_t size_x;
    rt_int32_t size_y;
    rt_uint32_t settle_delay_usecs;
    rt_uint32_t penirq_recheck_delay_usecs;
    rt_uint32_t x_plate_ohms;
    rt_uint32_t pressure_max;
    rt_uint32_t settle_samples;
    rt_size_t transfer_len;
    rt_uint8_t *tx_buf;
    rt_uint8_t *rx_buf;
    rt_bool_t swap_xy;
    rt_bool_t invert_x;
    rt_bool_t invert_y;
    rt_bool_t keep_vref_on;
    rt_bool_t touching;
    rt_bool_t initialized;
    rt_uint32_t spi_errors;
    rt_ssize_t last_spi_len;
    rt_uint16_t last_x;
    rt_uint16_t last_y;
    rt_uint16_t last_pressure;
};

struct tsc2046_sample
{
    rt_int32_t x;
    rt_int32_t y;
    rt_int32_t z1;
    rt_int32_t z2;
    rt_uint32_t pressure;
};

#endif /* __TSC2046_H__ */
