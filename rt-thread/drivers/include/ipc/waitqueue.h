/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __WAITQUEUE_H__
#define __WAITQUEUE_H__

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>

#define RT_WQ_FLAG_CLEAN  0x00
#define RT_WQ_FLAG_WAKEUP 0x01

struct rt_wqueue_node;
typedef int (*rt_wqueue_func_t)(struct rt_wqueue_node *wait, void *key);

struct rt_wqueue_node
{
    rt_thread_t polling_thread;
    rt_list_t list;

    rt_wqueue_t *wqueue;
    rt_wqueue_func_t wakeup;
    rt_uint32_t key;
};
typedef struct rt_wqueue_node rt_wqueue_node_t;

typedef int (*__kpi___wqueue_default_wake)(
    struct rt_wqueue_node *wait, void *key);

rt_inline void rt_wqueue_init(rt_wqueue_t *queue)
{
    RT_ASSERT(queue != RT_NULL);

    queue->flag = RT_WQ_FLAG_CLEAN;
    rt_list_init(&(queue->waiting_list));
    rt_spin_lock_init(&(queue->spinlock));
}

typedef int (*__kpi_rt_wqueue_wait_interruptible)(
    rt_wqueue_t *queue, int condition, int timeout);

typedef void (*__kpi_rt_wqueue_add)(
    rt_wqueue_t *queue, struct rt_wqueue_node *node);
typedef void (*__kpi_rt_wqueue_remove)(struct rt_wqueue_node *node);
typedef int (*__kpi_rt_wqueue_wait)(
    rt_wqueue_t *queue, int condition, int timeout);
typedef int (*__kpi_rt_wqueue_wait_killable)(
    rt_wqueue_t *queue, int condition, int timeout);
typedef void (*__kpi_rt_wqueue_wakeup)(rt_wqueue_t *queue, void *key);
typedef void (*__kpi_rt_wqueue_wakeup_all)(rt_wqueue_t *queue, void *key);

#define DEFINE_WAIT_FUNC(name, function)                                       \
    struct rt_wqueue_node name = { rt_current_thread,                          \
        RT_LIST_OBJECT_INIT(((name).list)), function, 0 }

#define DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, __wqueue_default_wake)

KPI_EXTERN(__wqueue_default_wake);
KPI_EXTERN(rt_wqueue_add);
KPI_EXTERN(rt_wqueue_remove);
KPI_EXTERN(rt_wqueue_wait);
KPI_EXTERN(rt_wqueue_wait_killable);
KPI_EXTERN(rt_wqueue_wait_interruptible);
KPI_EXTERN(rt_wqueue_wakeup);
KPI_EXTERN(rt_wqueue_wakeup_all);

#endif /* __WAITQUEUE_H__ */
