/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SITRONIX_TS_I2C_H__
#define __SITRONIX_TS_I2C_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <sitronix_ts.h>

struct sitronix_ts_i2c_data
{
    struct rt_i2c_bus_device *client;
    rt_uint32_t client_addr;
};

#endif /* __SITRONIX_TS_I2C_H__ */
