/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __OFW_IO_H__
#define __OFW_IO_H__

#include <drivers/ofw.h>
// #include <ioremap.h>
#include <kpi.h>

typedef int (*__kpi_rt_ofw_bus_addr_cells)(struct rt_ofw_node *np);
typedef int (*__kpi_rt_ofw_bus_size_cells)(struct rt_ofw_node *np);
typedef int (*__kpi_rt_ofw_io_addr_cells)(struct rt_ofw_node *np);
typedef int (*__kpi_rt_ofw_io_size_cells)(struct rt_ofw_node *np);

typedef int (*__kpi_rt_ofw_get_address_count)(struct rt_ofw_node *np);
typedef rt_err_t (*__kpi_rt_ofw_get_address)(struct rt_ofw_node *np,
    int index,
    rt_uint64_t *out_address,
    rt_uint64_t *out_size);
typedef rt_err_t (*__kpi_rt_ofw_get_address_by_name)(struct rt_ofw_node *np,
    const char *name,
    rt_uint64_t *out_address,
    rt_uint64_t *out_size);
typedef int (*__kpi_rt_ofw_get_address_array)(
    struct rt_ofw_node *np, int nr, rt_uint64_t *out_regs);

typedef rt_uint64_t (*__kpi_rt_ofw_translate_address)(
    struct rt_ofw_node *np, const char *range_type, rt_uint64_t address);
typedef rt_uint64_t (*__kpi_rt_ofw_reverse_address)(
    struct rt_ofw_node *np, const char *range_type, rt_uint64_t address);

typedef void *(*__kpi_rt_ofw_iomap)(struct rt_ofw_node *np, int index);
typedef void *(*__kpi_rt_ofw_iomap_by_name)(
    struct rt_ofw_node *np, const char *name);

KPI_EXTERN(rt_ofw_bus_addr_cells);
KPI_EXTERN(rt_ofw_bus_size_cells);
KPI_EXTERN(rt_ofw_io_addr_cells);
KPI_EXTERN(rt_ofw_io_size_cells);
KPI_EXTERN(rt_ofw_get_address_count);
KPI_EXTERN(rt_ofw_get_address);
KPI_EXTERN(rt_ofw_get_address_by_name);
KPI_EXTERN(rt_ofw_get_address_array);
KPI_EXTERN(rt_ofw_translate_address);
KPI_EXTERN(rt_ofw_reverse_address);
KPI_EXTERN(rt_ofw_iomap);
KPI_EXTERN(rt_ofw_iomap_by_name);

rt_inline rt_uint64_t rt_ofw_translate_dma2cpu(
    struct rt_ofw_node *np, rt_uint64_t address)
{
    rt_uint64_t bus_addr, cpu_addr;

    bus_addr = rt_ofw_reverse_address(np, "dma-ranges", address);
    cpu_addr = rt_ofw_translate_address(np, "ranges", bus_addr);

    return cpu_addr != ~0ULL ? cpu_addr : address;
}

rt_inline rt_uint64_t rt_ofw_translate_cpu2dma(
    struct rt_ofw_node *np, rt_uint64_t address)
{
    rt_uint64_t bus_addr, dma_addr;

    bus_addr = rt_ofw_reverse_address(np, "ranges", address);
    dma_addr = rt_ofw_translate_address(np, "dma-ranges", bus_addr);

    return dma_addr != ~0ULL ? dma_addr : address;
}

#endif /* __OFW_IO_H__ */
