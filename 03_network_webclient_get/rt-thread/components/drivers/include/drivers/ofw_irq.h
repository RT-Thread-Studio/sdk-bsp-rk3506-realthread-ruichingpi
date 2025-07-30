/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __OFW_IRQ_H__
#define __OFW_IRQ_H__

#include <drivers/ofw.h>
#include <kpi.h>

typedef int (*__kpi_rt_ofw_irq_cells)(struct rt_ofw_node *np);

typedef rt_err_t (*__kpi_rt_ofw_parse_irq_map)(
    struct rt_ofw_node *np, struct rt_ofw_cell_args *irq_args);
typedef rt_err_t (*__kpi_rt_ofw_parse_irq_cells)(
    struct rt_ofw_node *np, int index, struct rt_ofw_cell_args *out_irq_args);

typedef struct rt_ofw_node *(*__kpi_rt_ofw_find_irq_parent)(
    struct rt_ofw_node *np, int *out_interrupt_cells);
typedef int (*__kpi_rt_ofw_map_irq)(struct rt_ofw_cell_args *irq_args);

typedef int (*__kpi_rt_ofw_get_irq_count)(struct rt_ofw_node *np);
typedef int (*__kpi_rt_ofw_get_irq)(struct rt_ofw_node *np, int index);
typedef int (*__kpi_rt_ofw_get_irq_by_name)(
    struct rt_ofw_node *np, const char *name);

KPI_EXTERN(rt_ofw_irq_cells);
KPI_EXTERN(rt_ofw_parse_irq_map);
KPI_EXTERN(rt_ofw_parse_irq_cells);
KPI_EXTERN(rt_ofw_find_irq_parent);
KPI_EXTERN(rt_ofw_map_irq);
KPI_EXTERN(rt_ofw_get_irq_count);
KPI_EXTERN(rt_ofw_get_irq);
KPI_EXTERN(rt_ofw_get_irq_by_name);

#endif /* __OFW_IRQ_H__ */
