/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RT_PERF_TESTCASE_H__
#define __RT_PERF_TESTCASE_H__

#include "rt_perf.h"
#include <rtdef.h>

rt_err_t rt_perf_thread_jitter(struct rt_perf *perf);

#if defined(RT_PERF_ENABLE_IRQ_LATENCY)
rt_err_t rt_perf_irq_latency(struct rt_perf *perf);
#endif /* RT_PERF_ENABLE_IRQ_LATENCY */

int rt_perf(int argc, char *argv[]);

#endif /* __RT_PERF_TESTCASE_H__ */
