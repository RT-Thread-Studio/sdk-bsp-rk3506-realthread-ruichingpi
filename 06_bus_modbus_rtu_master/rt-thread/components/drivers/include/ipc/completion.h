/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __COMPLETION_H_
#define __COMPLETION_H_

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>

/**
 * RT-Completion - A Tiny(resource-constrained) & Rapid(lockless) IPC Primitive
 *
 * It's an IPC using one pointer word with the encoding:
 *
 * BIT      | MAX-1 ----------------- 1 |       0        |
 * CONTENT  |   suspended_thread & ~1   | completed flag |
 */

struct rt_completion
{
    /* suspended thread, and completed flag */
    rt_atomic_t susp_thread_n_flag;
};

#define RT_COMPLETION_INIT(comp) { 0 }

typedef void (*__kpi_rt_completion_init)(struct rt_completion *completion);
typedef rt_err_t (*__kpi_rt_completion_wait)(
    struct rt_completion *completion, rt_int32_t timeout);
typedef rt_err_t (*__kpi_rt_completion_wait_noisr)(
    struct rt_completion *completion, rt_int32_t timeout);
typedef rt_err_t (*__kpi_rt_completion_wait_flags)(
    struct rt_completion *completion, rt_int32_t timeout, int suspend_flag);
typedef rt_err_t (*__kpi_rt_completion_wait_flags_noisr)(
    struct rt_completion *completion, rt_int32_t timeout, int suspend_flag);
typedef void (*__kpi_rt_completion_done)(struct rt_completion *completion);
typedef rt_err_t (*__kpi_rt_completion_wakeup)(
    struct rt_completion *completion);
typedef rt_err_t (*__kpi_rt_completion_wakeup_by_errno)(
    struct rt_completion *completion, rt_err_t error);

KPI_EXTERN(rt_completion_init);
KPI_EXTERN(rt_completion_wait);
KPI_EXTERN(rt_completion_wait_noisr);
KPI_EXTERN(rt_completion_wait_flags);
KPI_EXTERN(rt_completion_wait_flags_noisr);
KPI_EXTERN(rt_completion_done);
KPI_EXTERN(rt_completion_wakeup);
KPI_EXTERN(rt_completion_wakeup_by_errno);

#endif /* __COMPLETION_H_ */
