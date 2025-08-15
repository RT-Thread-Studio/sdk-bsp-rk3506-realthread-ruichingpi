/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-18     ChenYong     First version
 */

#ifndef __NETDEV_H__
#define __NETDEV_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*__kpi_if_set_mac)(char *name, uint8_t *mac);
typedef int (*__kpi_if_get_mac)(char *name, uint8_t *mac);
typedef int (*__kpi_if_set_dns)(char *netdev_name, uint8_t dns_num, char *dns_server);
typedef int (*__kpi_if_get_dns)(char *netdev_name, uint8_t dns_num, char *dns_server);
typedef int (*__kpi_if_dhcp)(char *name, int enable);
typedef int (*__kpi_if_set_ip)(char *name, char *ip_addr, char *gw_addr, char *nm_addr);
typedef int (*__kpi_if_get_ip)(char *name, char *ip_addr, char *gw_addr, char *nm_addr);
typedef int (*__kpi_if_up)(char *name);
typedef int (*__kpi_if_down)(char *name);

KPI_EXTERN(if_set_mac);
KPI_EXTERN(if_get_mac);
KPI_EXTERN(if_set_dns);
KPI_EXTERN(if_get_dns);
KPI_EXTERN(if_dhcp);
KPI_EXTERN(if_set_ip);
KPI_EXTERN(if_get_ip);
KPI_EXTERN(if_up);
KPI_EXTERN(if_down);

#ifdef __cplusplus
}
#endif

#endif /* __NETDEV_H__ */
