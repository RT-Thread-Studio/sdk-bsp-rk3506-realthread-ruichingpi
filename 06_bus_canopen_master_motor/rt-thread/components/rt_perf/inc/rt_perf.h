/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RT_PERF_H__
#define __RT_PERF_H__

#include <rtdef.h>

#define THREAD_SIZE      4096
#define THREAD_PRIORITY  5
#define THREAD_TIMESLICE 20

#define PERF_COUNT       10000
#define MEM_LOOP         10
#define MEMORY_SIZE      (1024 * 1024)
#define KB2MB(size)      (size / (1024 * 1024))
#define PERF_CPU         (2)

#define FILE_NAME        "filespeed.txt"
#define EXPORT_DATA_NAME "data.csv"

struct rt_perf
{
    char name[32];
    rt_bool_t info_head;
    rt_bool_t timer_dir;
#ifdef RT_PERF_EXPORT_CSV_DATA
    rt_uint32_t *value;
#endif /* RT_PERF_EXPORT_CSV_DATA */
    volatile rt_uint32_t timer_frq;
    volatile rt_uint32_t begin_time;
    volatile rt_uint32_t real_time;
    volatile rt_uint32_t tot_time;
    volatile rt_uint32_t max_time;
    volatile rt_uint32_t min_time;
    volatile rt_uint32_t count;
    volatile double avg_time;

    void (*manual_change)(struct rt_perf *perf);
};

void rt_perf_start(struct rt_perf *perf);
void rt_perf_stop(struct rt_perf *perf);
void rt_perf_dump(struct rt_perf *perf);
void rt_perf_clear(struct rt_perf *perf);
void rt_perf_manual_set(
    struct rt_perf *perf, void (*manual_change)(struct rt_perf *perf));

#endif /* __RT_PERF_H__ */
