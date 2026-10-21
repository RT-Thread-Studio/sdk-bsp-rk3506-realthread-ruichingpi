/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_usage.h"

#define CPU_USAGE_SAMPLE_MS        200U
#define CPU_USAGE_MIN_SAMPLE_MS     10U
#define CPU_USAGE_MAX_SAMPLE_MS  10000U
#define CPU_TARGET_ALL                -1

typedef struct
{
    rt_uint32_t total[RT_CPUS_NR];
    rt_uint32_t idle[RT_CPUS_NR];
} cpu_usage_counters_t;

/* Serialize counter updates to give every CPU the same snapshot boundary. */
static RT_DEFINE_SPINLOCK(cpu_usage_lock);
static volatile rt_uint32_t total_ticks[RT_CPUS_NR];
static volatile rt_uint32_t idle_ticks[RT_CPUS_NR];
static rt_bool_t cpu_usage_initialized;

static void cpu_usage_tick_hook(void)
{
    int cpu_id = rt_hw_cpu_id();
    rt_base_t level;

    if (cpu_id < 0 || cpu_id >= RT_CPUS_NR)
    {
        return;
    }

    level = rt_spin_lock_irqsave(&cpu_usage_lock);
    total_ticks[cpu_id]++;
    if (rt_thread_self() == rt_thread_idle_gethandler())
    {
        idle_ticks[cpu_id]++;
    }
    rt_spin_unlock_irqrestore(&cpu_usage_lock, level);
}

static void cpu_usage_take_snapshot(cpu_usage_counters_t *snapshot)
{
    rt_base_t level = rt_spin_lock_irqsave(&cpu_usage_lock);
    int i;

    if (!cpu_usage_initialized)
    {
        rt_memset((void *)total_ticks, 0, sizeof(total_ticks));
        rt_memset((void *)idle_ticks, 0, sizeof(idle_ticks));
        rt_tick_sethook(cpu_usage_tick_hook);
        cpu_usage_initialized = RT_TRUE;
    }

    for (i = 0; i < RT_CPUS_NR; i++)
    {
        snapshot->total[i] = total_ticks[i];
        snapshot->idle[i] = idle_ticks[i];
    }

    rt_spin_unlock_irqrestore(&cpu_usage_lock, level);
}

static rt_uint8_t cpu_usage_calc(rt_uint32_t total, rt_uint32_t idle)
{
    return (rt_uint8_t)(((total - idle) * 100U + total / 2U) / total);
}

rt_err_t get_cpu_usage(rt_uint32_t sample_ms,
    rt_uint8_t usage[RT_CPUS_NR])
{
    cpu_usage_counters_t start;
    cpu_usage_counters_t end;
    rt_uint8_t result[RT_CPUS_NR];
    int i;

    if (usage == RT_NULL || sample_ms < CPU_USAGE_MIN_SAMPLE_MS ||
        sample_ms > CPU_USAGE_MAX_SAMPLE_MS)
    {
        return -RT_EINVAL;
    }

    cpu_usage_take_snapshot(&start);
    rt_thread_mdelay(sample_ms);
    cpu_usage_take_snapshot(&end);

    for (i = 0; i < RT_CPUS_NR; i++)
    {
        rt_uint32_t total_delta = end.total[i] - start.total[i];
        rt_uint32_t idle_delta = end.idle[i] - start.idle[i];

        if (total_delta == 0)
        {
            return -RT_ETIMEOUT;
        }
        if (idle_delta > total_delta)
        {
            idle_delta = total_delta;
        }

        result[i] = cpu_usage_calc(total_delta, idle_delta);
    }

    rt_memcpy(usage, result, sizeof(result));
    return RT_EOK;
}

static rt_err_t parse_uint(const char *text, rt_uint32_t min,
    rt_uint32_t max, rt_uint32_t *value)
{
    char *end;
    unsigned long parsed;

    if (text == RT_NULL || *text == '\0' || value == RT_NULL)
    {
        return -RT_EINVAL;
    }

    parsed = strtoul(text, &end, 10);
    if (*end != '\0' || parsed < min || parsed > max)
    {
        return -RT_EINVAL;
    }

    *value = (rt_uint32_t)parsed;
    return RT_EOK;
}

static void cpu_usage(int argc, char **argv)
{
    rt_uint32_t sample_ms = CPU_USAGE_SAMPLE_MS;
    rt_uint8_t usage[RT_CPUS_NR];
    rt_err_t ret;
    int target = CPU_TARGET_ALL;
    int i;

    if (argc > 3)
    {
        goto usage;
    }

    if (argc >= 2)
    {
        rt_uint32_t value;

        if (strcmp(argv[1], "all") == 0)
        {
            target = CPU_TARGET_ALL;
        }
        else if (parse_uint(argv[1], 0, RT_CPUS_NR - 1, &value) == RT_EOK)
        {
            target = (int)value;
        }
        else if (parse_uint(argv[1], CPU_USAGE_MIN_SAMPLE_MS,
            CPU_USAGE_MAX_SAMPLE_MS, &value) == RT_EOK)
        {
            sample_ms = value;
        }
        else
        {
            goto usage;
        }
    }

    if (argc == 3 && parse_uint(argv[2], CPU_USAGE_MIN_SAMPLE_MS,
        CPU_USAGE_MAX_SAMPLE_MS, &sample_ms) != RT_EOK)
    {
        goto usage;
    }

    ret = get_cpu_usage(sample_ms, usage);
    if (ret != RT_EOK)
    {
        rt_kprintf("cpu usage sample failed: %d\n", ret);
        return;
    }

    rt_kprintf("%-5s %s\n", "CPU", "Usage");
    if (target == CPU_TARGET_ALL)
    {
        for (i = 0; i < RT_CPUS_NR; i++)
        {
            rt_kprintf("CPU%d:  %u%%\n", i, usage[i]);
        }
    }
    else
    {
        rt_kprintf("CPU%d:  %u%%\n", target, usage[target]);
    }
    return;

usage:
    rt_kprintf("usage: cpu_usage [all|cpu_id] [sample_ms]\n");
}
MSH_CMD_EXPORT(cpu_usage, get per-CPU usage);
