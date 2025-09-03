/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <drivers/core/driver.h>
#include <drivers/core/device.h>
#include <drivers/ofw.h>
#include <kpi.h>

struct rt_platform_device
{
    struct rt_device parent;
    struct device dev;

    int dev_id;

    const char *name;
    const struct rt_ofw_node_id *id;

    void *priv;
};

struct rt_platform_driver
{
    struct rt_driver parent;

    const char *name;
    const struct rt_ofw_node_id *ids;

    rt_err_t (*probe)(struct rt_platform_device *pdev);
    rt_err_t (*remove)(struct rt_platform_device *pdev);
    rt_err_t (*shutdown)(struct rt_platform_device *pdev);
};

typedef struct rt_platform_device *(*__kpi_rt_platform_device_alloc)(
    const char *name);

typedef rt_err_t (*__kpi_rt_platform_driver_register)(
    struct rt_platform_driver *pdrv);
typedef rt_err_t (*__kpi_rt_platform_device_register)(
    struct rt_platform_device *pdev);

typedef rt_err_t (*__kpi_rt_platform_ofw_device_probe_child)(
    struct rt_ofw_node *np);
typedef rt_err_t (*__kpi_rt_platform_ofw_request)(struct rt_ofw_node *np);
typedef rt_err_t (*__kpi_rt_platform_ofw_free)(struct rt_platform_device *pdev);

#define RT_PLATFORM_DRIVER_EXPORT(driver)                                      \
    RT_DRIVER_EXPORT(driver, platform, BUILIN)

KPI_EXTERN(rt_platform_device_alloc);
KPI_EXTERN(rt_platform_driver_register);
KPI_EXTERN(rt_platform_device_register);
KPI_EXTERN(rt_platform_ofw_device_probe_child);
KPI_EXTERN(rt_platform_ofw_request);
KPI_EXTERN(rt_platform_ofw_free);

#endif /* __PLATFORM_H__ */
