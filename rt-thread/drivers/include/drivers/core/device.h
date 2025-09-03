/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <rtdef.h>

struct device
{
    struct rt_object parent;
    rt_uint32_t ref_count;

    struct rt_driver *drv;
    struct rt_bus *bus;
    void *ofw_node;
    rt_list_t node;

    void *user_data;
};

#endif /* __DEVICE_H__ */
