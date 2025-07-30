/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DATAQUEUE_H__
#define __DATAQUEUE_H__

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>

#define RT_DATAQUEUE_EVENT_UNKNOWN 0x00
#define RT_DATAQUEUE_EVENT_POP     0x01
#define RT_DATAQUEUE_EVENT_PUSH    0x02
#define RT_DATAQUEUE_EVENT_LWM     0x03

struct rt_data_item;

/* data queue implementation */
struct rt_data_queue
{
    rt_uint32_t magic;

    rt_uint16_t size;
    rt_uint16_t lwm;

    rt_uint16_t get_index : 15;
    rt_uint16_t is_empty  : 1;
    rt_uint16_t put_index : 15;
    rt_uint16_t is_full   : 1;

    struct rt_data_item *queue;
    struct rt_spinlock spinlock;

    rt_list_t suspended_push_list;
    rt_list_t suspended_pop_list;

    /* event notify */
    void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event);
};

/**
 * DataQueue for DeviceDriver
 */
typedef rt_err_t (*__kpi_rt_data_queue_init)(struct rt_data_queue *queue,
    rt_uint16_t size,
    rt_uint16_t lwm,
    void (*evt_notify)(struct rt_data_queue *queue, rt_uint32_t event));
typedef rt_err_t (*__kpi_rt_data_queue_push)(struct rt_data_queue *queue,
    const void *data_ptr,
    rt_size_t data_size,
    rt_int32_t timeout);
typedef rt_err_t (*__kpi_rt_data_queue_pop)(struct rt_data_queue *queue,
    const void **data_ptr,
    rt_size_t *size,
    rt_int32_t timeout);
typedef rt_err_t (*__kpi_rt_data_queue_peek)(
    struct rt_data_queue *queue, const void **data_ptr, rt_size_t *size);
typedef void (*__kpi_rt_data_queue_reset)(struct rt_data_queue *queue);
typedef rt_err_t (*__kpi_rt_data_queue_deinit)(struct rt_data_queue *queue);
typedef rt_uint16_t (*__kpi_rt_data_queue_len)(struct rt_data_queue *queue);

KPI_EXTERN(rt_data_queue_init);
KPI_EXTERN(rt_data_queue_push);
KPI_EXTERN(rt_data_queue_pop);
KPI_EXTERN(rt_data_queue_peek);
KPI_EXTERN(rt_data_queue_reset);
KPI_EXTERN(rt_data_queue_deinit);
KPI_EXTERN(rt_data_queue_len);

#endif /* __DATAQUEUE_H__ */
