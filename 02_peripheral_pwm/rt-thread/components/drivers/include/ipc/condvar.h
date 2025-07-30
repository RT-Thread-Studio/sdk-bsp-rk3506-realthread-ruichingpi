/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __CONDVAR_H__
#define __CONDVAR_H__

#include <rtthread.h>
#include <kpi.h>

typedef struct rt_condvar
{
#ifdef USING_RT_OBJECT
    struct rt_object parent;
#endif
    rt_atomic_t waiters_cnt;
    rt_atomic_t waiting_mtx;
    struct rt_wqueue event;
} *rt_condvar_t;

typedef void (*__kpi_rt_condvar_init)(rt_condvar_t cv, char *name);
typedef int (*__kpi_rt_condvar_timedwait)(
    rt_condvar_t cv, rt_mutex_t mtx, int suspend_flag, rt_tick_t timeout);
typedef int (*__kpi_rt_condvar_signal)(rt_condvar_t cv);
typedef int (*__kpi_rt_condvar_broadcast)(rt_condvar_t cv);

rt_inline void rt_condvar_detach(rt_condvar_t cv)
{
    RT_UNUSED(cv);
    return;
}

KPI_EXTERN(rt_condvar_init);
KPI_EXTERN(rt_condvar_timedwait);
KPI_EXTERN(rt_condvar_signal);
KPI_EXTERN(rt_condvar_broadcast);

#endif /* __CONDVAR_H__ */
