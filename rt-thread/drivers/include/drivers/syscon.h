/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SYSCON_H__
#define __SYSCON_H__

#include <drivers/ofw.h>
#include <kpi.h>

struct rt_syscon
{
    rt_list_t list;

    struct rt_ofw_node *np;

    void *iomem_base;
    rt_size_t iomem_size;
    struct rt_spinlock rw_lock;
};

typedef struct rt_syscon *(*__kpi_rt_syscon_find_by_ofw_phandle)(
    struct rt_ofw_node *np, const char *propname);

KPI_EXTERN(rt_syscon_find_by_ofw_phandle);

#endif /* __SYSCON_H__ */
