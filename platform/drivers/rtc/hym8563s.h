/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __HYM8563S_H__
#define __HYM8563S_H__

#include <rtthread.h>
#include <rtdevice.h>

void hym8563s_get_time(struct rt_i2c_bus_device *i2c_dev, rt_uint32_t addr,
    time_t *r_time);
void hym8563s_set_time(struct rt_i2c_bus_device *i2c_dev, rt_uint32_t
    addr, time_t *r_time);

#endif /* __HYM8563S_H__ */
