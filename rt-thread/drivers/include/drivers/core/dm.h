/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DM_H__
#define __DM_H__

#include <bitmap.h>
#include <drivers/byteorder.h>
#include <drivers/core/master_id.h>
#include <drivers/misc.h>
// #include <ioremap.h>
#include <kpi.h>
#include <rtdef.h>
#include <rthw.h>

#ifndef RT_CPUS_NR
#define RT_CPUS_NR 1
#endif

#ifndef RT_USING_SMP
extern int rt_hw_cpu_id(void);
#endif

void rt_dm_secondary_cpu_init(void);

/* ID Allocation */
struct rt_dm_ida
{
    rt_uint8_t master_id;

#define RT_DM_IDA_NUM 256
    RT_BITMAP_DECLARE(map, RT_DM_IDA_NUM);
    struct rt_spinlock lock;
};

#define RT_DM_IDA_INIT(id) { .master_id = MASTER_ID_##id }

typedef int (*__kpi_rt_dm_ida_alloc)(struct rt_dm_ida *ida);
typedef rt_bool_t (*__kpi_rt_dm_ida_take)(struct rt_dm_ida *ida, int id);
typedef void (*__kpi_rt_dm_ida_free)(struct rt_dm_ida *ida, int id);

typedef rt_device_t (*__kpi_rt_dm_device_find)(int master_id, int device_id);

typedef int (*__kpi_rt_dm_dev_set_name_auto)(
    rt_device_t dev, const char *prefix);
typedef int (*__kpi_rt_dm_dev_get_name_id)(rt_device_t dev);

typedef int (*__kpi_rt_dm_dev_set_name)(
    rt_device_t dev, const char *format, ...);
typedef const char *(*__kpi_rt_dm_dev_get_name)(rt_device_t dev);

typedef int (*__kpi_rt_dm_dev_get_address_count)(rt_device_t dev);
typedef rt_err_t (*__kpi_rt_dm_dev_get_address)(rt_device_t dev,
    int index,
    rt_uint64_t *out_address,
    rt_uint64_t *out_size);
typedef rt_err_t (*__kpi_rt_dm_dev_get_address_by_name)(rt_device_t dev,
    const char *name,
    rt_uint64_t *out_address,
    rt_uint64_t *out_size);
typedef int (*__kpi_rt_dm_dev_get_address_array)(
    rt_device_t dev, int nr, rt_uint64_t *out_regs);

typedef void *(*__kpi_rt_dm_dev_iomap)(rt_device_t dev, int index);
typedef void *(*__kpi_rt_dm_dev_iomap_by_name)(
    rt_device_t dev, const char *name);

typedef int (*__kpi_rt_dm_dev_get_irq_count)(rt_device_t dev);
typedef int (*__kpi_rt_dm_dev_get_irq)(rt_device_t dev, int index);
typedef int (*__kpi_rt_dm_dev_get_irq_by_name)(
    rt_device_t dev, const char *name);

typedef void (*__kpi_rt_dm_dev_bind_fwdata)(
    rt_device_t dev, void *fw_np, void *data);
typedef void (*__kpi_rt_dm_dev_unbind_fwdata)(rt_device_t dev, void *fw_np);

typedef int (*__kpi_rt_dm_dev_prop_read_u8_array_index)(rt_device_t dev,
    const char *propname,
    int index,
    int nr,
    rt_uint8_t *out_values);
typedef int (*__kpi_rt_dm_dev_prop_read_u16_array_index)(rt_device_t dev,
    const char *propname,
    int index,
    int nr,
    rt_uint16_t *out_values);
typedef int (*__kpi_rt_dm_dev_prop_read_u32_array_index)(rt_device_t dev,
    const char *propname,
    int index,
    int nr,
    rt_uint32_t *out_values);
typedef int (*__kpi_rt_dm_dev_prop_read_u64_array_index)(rt_device_t dev,
    const char *propname,
    int index,
    int nr,
    rt_uint64_t *out_values);
typedef int (*__kpi_rt_dm_dev_prop_read_string_array_index)(rt_device_t dev,
    const char *propname,
    int index,
    int nr,
    const char **out_strings);

typedef int (*__kpi_rt_dm_dev_prop_count_of_size)(
    rt_device_t dev, const char *propname, int size);
typedef int (*__kpi_rt_dm_dev_prop_index_of_string)(
    rt_device_t dev, const char *propname, const char *string);

typedef rt_bool_t (*__kpi_rt_dm_dev_prop_read_bool)(
    rt_device_t dev, const char *propname);

KPI_EXTERN(rt_dm_ida_alloc);
KPI_EXTERN(rt_dm_ida_take);
KPI_EXTERN(rt_dm_ida_free);
KPI_EXTERN(rt_dm_device_find);
KPI_EXTERN(rt_dm_dev_set_name_auto);
KPI_EXTERN(rt_dm_dev_get_name_id);
KPI_EXTERN(rt_dm_dev_set_name);
KPI_EXTERN(rt_dm_dev_get_name);
KPI_EXTERN(rt_dm_dev_get_address_count);
KPI_EXTERN(rt_dm_dev_get_address);
KPI_EXTERN(rt_dm_dev_get_address_by_name);
KPI_EXTERN(rt_dm_dev_get_address_array);
KPI_EXTERN(rt_dm_dev_iomap);
KPI_EXTERN(rt_dm_dev_iomap_by_name);
KPI_EXTERN(rt_dm_dev_get_irq_count);
KPI_EXTERN(rt_dm_dev_get_irq);
KPI_EXTERN(rt_dm_dev_get_irq_by_name);
KPI_EXTERN(rt_dm_dev_bind_fwdata);
KPI_EXTERN(rt_dm_dev_unbind_fwdata);
KPI_EXTERN(rt_dm_dev_prop_read_u8_array_index);
KPI_EXTERN(rt_dm_dev_prop_read_u16_array_index);
KPI_EXTERN(rt_dm_dev_prop_read_u32_array_index);
KPI_EXTERN(rt_dm_dev_prop_read_u64_array_index);
KPI_EXTERN(rt_dm_dev_prop_read_string_array_index);
KPI_EXTERN(rt_dm_dev_prop_count_of_size);
KPI_EXTERN(rt_dm_dev_prop_index_of_string);
KPI_EXTERN(rt_dm_dev_prop_read_bool);

rt_inline rt_err_t rt_dm_dev_prop_read_u8_index(
    rt_device_t dev, const char *propname, int index, rt_uint8_t *out_value)
{
    int nr =
        rt_dm_dev_prop_read_u8_array_index(dev, propname, index, 1, out_value);

    return nr > 0 ? RT_EOK : (rt_err_t)nr;
}

rt_inline rt_err_t rt_dm_dev_prop_read_u16_index(
    rt_device_t dev, const char *propname, int index, rt_uint16_t *out_value)
{
    int nr =
        rt_dm_dev_prop_read_u16_array_index(dev, propname, index, 1, out_value);

    return nr > 0 ? RT_EOK : (rt_err_t)nr;
}

rt_inline rt_err_t rt_dm_dev_prop_read_u32_index(
    rt_device_t dev, const char *propname, int index, rt_uint32_t *out_value)
{
    int nr =
        rt_dm_dev_prop_read_u32_array_index(dev, propname, index, 1, out_value);

    return nr > 0 ? RT_EOK : (rt_err_t)nr;
}

rt_inline rt_err_t rt_dm_dev_prop_read_u64_index(
    rt_device_t dev, const char *propname, int index, rt_uint64_t *out_value)
{
    int nr =
        rt_dm_dev_prop_read_u64_array_index(dev, propname, index, 1, out_value);

    return nr > 0 ? RT_EOK : (rt_err_t)nr;
}

rt_inline rt_err_t rt_dm_dev_prop_read_string_index(
    rt_device_t dev, const char *propname, int index, const char **out_string)
{
    int nr = rt_dm_dev_prop_read_string_array_index(
        dev, propname, index, 1, out_string);

    return nr > 0 ? RT_EOK : (rt_err_t)nr;
}

rt_inline rt_err_t rt_dm_dev_prop_read_u8(
    rt_device_t dev, const char *propname, rt_uint8_t *out_value)
{
    return rt_dm_dev_prop_read_u8_index(dev, propname, 0, out_value);
}

rt_inline rt_err_t rt_dm_dev_prop_read_u16(
    rt_device_t dev, const char *propname, rt_uint16_t *out_value)
{
    return rt_dm_dev_prop_read_u16_index(dev, propname, 0, out_value);
}

rt_inline rt_err_t rt_dm_dev_prop_read_u32(
    rt_device_t dev, const char *propname, rt_uint32_t *out_value)
{
    return rt_dm_dev_prop_read_u32_index(dev, propname, 0, out_value);
}

rt_inline rt_err_t rt_dm_dev_prop_read_s32(
    rt_device_t dev, const char *propname, rt_int32_t *out_value)
{
    return rt_dm_dev_prop_read_u32_index(
        dev, propname, 0, (rt_uint32_t *)out_value);
}

rt_inline rt_err_t rt_dm_dev_prop_read_u64(
    rt_device_t dev, const char *propname, rt_uint64_t *out_value)
{
    return rt_dm_dev_prop_read_u64_index(dev, propname, 0, out_value);
}

rt_inline rt_err_t rt_dm_dev_prop_read_string(
    rt_device_t dev, const char *propname, const char **out_string)
{
    return rt_dm_dev_prop_read_string_index(dev, propname, 0, out_string);
}

rt_inline int rt_dm_dev_prop_count_of_u8(rt_device_t dev, const char *propname)
{
    return rt_dm_dev_prop_count_of_size(dev, propname, sizeof(rt_uint8_t));
}

rt_inline int rt_dm_dev_prop_count_of_u16(rt_device_t dev, const char *propname)
{
    return rt_dm_dev_prop_count_of_size(dev, propname, sizeof(rt_uint16_t));
}

rt_inline int rt_dm_dev_prop_count_of_u32(rt_device_t dev, const char *propname)
{
    return rt_dm_dev_prop_count_of_size(dev, propname, sizeof(rt_uint32_t));
}

rt_inline int rt_dm_dev_prop_count_of_u64(rt_device_t dev, const char *propname)
{
    return rt_dm_dev_prop_count_of_size(dev, propname, sizeof(rt_uint64_t));
}

#endif /* __DM_H__ */
