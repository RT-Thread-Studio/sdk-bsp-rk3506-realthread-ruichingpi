/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-22     xiangxistu integrate v1.4.1 v2.0.3 and v2.1.2 porting layer
 */

#ifndef __NETIF_ETH_DEVICE_H__
#define __NETIF_ETH_DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

// #include "lwip/netif.h"
#include <rtthread.h>

#define NIOCTL_GADDR        0x01
#ifndef RT_LWIP_ETH_MTU
#define ETHERNET_MTU        1500
#else
#define ETHERNET_MTU        RT_LWIP_ETH_MTU
#endif

/* eth flag with auto_linkup or phy_linkup */
#define ETHIF_LINK_AUTOUP   0x0000
#define ETHIF_LINK_PHYUP    0x0100

struct eth_device;

typedef void (*__kpi_rt_eth_device_deinit)(struct eth_device *dev);
typedef rt_err_t (*__kpi_rt_eth_device_ready)(struct eth_device* dev);
typedef rt_err_t (*__kpi_rt_eth_device_init)(struct eth_device * dev, const char *name);
typedef rt_err_t (*__kpi_rt_eth_device_init_with_flag)(struct eth_device *dev, const char *name, rt_uint16_t flag);
typedef rt_err_t (*__kpi_rt_eth_device_linkchange)(struct eth_device* dev, rt_bool_t up);

KPI_EXTERN(rt_eth_device_deinit);
KPI_EXTERN(rt_eth_device_ready);
KPI_EXTERN(rt_eth_device_init);
KPI_EXTERN(rt_eth_device_init_with_flag);
KPI_EXTERN(rt_eth_device_linkchange);

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_ETH_DEVICE_H__ */
