/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>
#include "completion.h"

enum
{
    RT_WORK_STATE_PENDING = 0x0001,    /* Work item pending state */
    RT_WORK_STATE_SUBMITTING = 0x0002, /* Work item submitting state */
};

/**
 * work type definitions
 */
enum
{
    RT_WORK_TYPE_DELAYED = 0x0001,
};

/* workqueue implementation */
struct rt_workqueue
{
    rt_list_t work_list;
    rt_list_t delayed_list;
    struct rt_work *work_current; /* current work */

    struct rt_semaphore sem;
    rt_thread_t work_thread;
    struct rt_spinlock spinlock;
    struct rt_completion wakeup_completion;
};

struct rt_work
{
    rt_list_t list;

    void (*work_func)(struct rt_work *work, void *work_data);
    void *work_data;
    rt_uint16_t flags;
    rt_uint16_t type;
    rt_tick_t timeout_tick;
    struct rt_workqueue *workqueue;
};

#ifdef RT_USING_HEAP
/**
 * WorkQueue for DeviceDriver
 */
typedef void (*__kpi_rt_work_init)(struct rt_work *work,
    void (*work_func)(struct rt_work *work, void *work_data),
    void *work_data);
typedef struct rt_workqueue *(*__kpi_rt_workqueue_create)(
    const char *name, rt_uint16_t stack_size, rt_uint8_t priority);
typedef rt_err_t (*__kpi_rt_workqueue_destroy)(struct rt_workqueue *queue);
typedef rt_err_t (*__kpi_rt_workqueue_dowork)(
    struct rt_workqueue *queue, struct rt_work *work);
typedef rt_err_t (*__kpi_rt_workqueue_submit_work)(
    struct rt_workqueue *queue, struct rt_work *work, rt_tick_t ticks);
typedef rt_err_t (*__kpi_rt_workqueue_cancel_work)(
    struct rt_workqueue *queue, struct rt_work *work);
typedef rt_err_t (*__kpi_rt_workqueue_cancel_work_sync)(
    struct rt_workqueue *queue, struct rt_work *work);
typedef rt_err_t (*__kpi_rt_workqueue_cancel_all_work)(
    struct rt_workqueue *queue);
typedef rt_err_t (*__kpi_rt_workqueue_urgent_work)(
    struct rt_workqueue *queue, struct rt_work *work);

#ifdef RT_USING_SYSTEM_WORKQUEUE
typedef rt_err_t (*__kpi_rt_work_submit)(struct rt_work *work, rt_tick_t ticks);
typedef rt_err_t (*__kpi_rt_work_urgent)(struct rt_work *work);
typedef rt_err_t (*__kpi_rt_work_cancel)(struct rt_work *work);
#endif /* RT_USING_SYSTEM_WORKQUEUE */
#endif /* RT_USING_HEAP */

KPI_EXTERN(rt_work_init);
KPI_EXTERN(rt_workqueue_create);
KPI_EXTERN(rt_workqueue_destroy);
KPI_EXTERN(rt_workqueue_dowork);
KPI_EXTERN(rt_workqueue_submit_work);
KPI_EXTERN(rt_workqueue_cancel_work);
KPI_EXTERN(rt_workqueue_cancel_work_sync);
KPI_EXTERN(rt_workqueue_cancel_all_work);
KPI_EXTERN(rt_workqueue_urgent_work);
KPI_EXTERN(rt_work_submit);
KPI_EXTERN(rt_work_urgent);
KPI_EXTERN(rt_work_cancel);

#endif /* __WORKQUEUE_H__ */
