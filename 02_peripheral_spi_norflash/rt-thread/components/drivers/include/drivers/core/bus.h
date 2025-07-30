/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __BUS_H__
#define __BUS_H__

#include <drivers/core/driver.h>
#include <kpi.h>
#include <rtdef.h>
#include <rthw.h>

typedef struct rt_bus *rt_bus_t;

struct rt_bus
{
    struct rt_object parent; /**< inherit from rt_object */

    const char *name;

    rt_list_t list;
    rt_list_t dev_list;
    rt_list_t drv_list;

    struct rt_spinlock dev_lock;
    struct rt_spinlock drv_lock;

    rt_bool_t (*match)(rt_driver_t drv, rt_device_t dev);
    rt_err_t (*probe)(rt_device_t dev);
    rt_err_t (*remove)(rt_device_t dev);
    rt_err_t (*shutdown)(rt_device_t dev);
};

typedef rt_err_t (*__kpi_rt_bus_for_each_dev)(
    rt_bus_t bus, void *data, int (*fn)(rt_device_t dev, void *));
typedef rt_err_t (*__kpi_rt_bus_for_each_drv)(
    rt_bus_t bus, void *data, int (*fn)(rt_driver_t drv, void *));

typedef rt_err_t (*__kpi_rt_bus_add_driver)(rt_bus_t bus, rt_driver_t drv);
typedef rt_err_t (*__kpi_rt_bus_add_device)(rt_bus_t bus, rt_device_t dev);
typedef rt_err_t (*__kpi_rt_bus_remove_driver)(rt_driver_t drv);
typedef rt_err_t (*__kpi_rt_bus_remove_device)(rt_device_t dev);

typedef rt_err_t (*__kpi_rt_bus_shutdown)(void);

typedef rt_bus_t (*__kpi_rt_bus_find_by_name)(const char *name);
typedef rt_err_t (*__kpi_rt_bus_reload_driver_device)(
    rt_bus_t new_bus, rt_device_t dev);

typedef rt_err_t (*__kpi_rt_bus_register)(rt_bus_t bus);

KPI_EXTERN(rt_bus_for_each_dev);
KPI_EXTERN(rt_bus_for_each_drv);
KPI_EXTERN(rt_bus_add_driver);
KPI_EXTERN(rt_bus_add_device);
KPI_EXTERN(rt_bus_remove_driver);
KPI_EXTERN(rt_bus_remove_device);
KPI_EXTERN(rt_bus_shutdown);
KPI_EXTERN(rt_bus_find_by_name);
KPI_EXTERN(rt_bus_reload_driver_device);
KPI_EXTERN(rt_bus_register);

#endif /* __BUS_H__ */
