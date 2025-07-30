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
#include "rt_perf_testcase.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CPUS               RT_CPUS_NR
#define LOAD_THREAD_STACK_SIZE 1024
#define LOAD_THREAD_PRIORITY   25

typedef struct
{
    int cpu_id;
    int load_percent;
    rt_thread_t thread;
    rt_atomic_t running;
} cpu_load_control_t;

static cpu_load_control_t cpu_loads[MAX_CPUS];

static void load_entry(void *param)
{
    cpu_load_control_t *ctrl = (cpu_load_control_t *)param;

    while (ctrl->running)
    {
        rt_tick_t start = rt_tick_get();
        rt_tick_t busy_ticks = (RT_TICK_PER_SECOND * ctrl->load_percent) / 100;
        rt_tick_t total_ticks = RT_TICK_PER_SECOND;

        while (rt_tick_get() - start < busy_ticks) { __asm volatile("nop"); }

        rt_thread_mdelay(
            (total_ticks - busy_ticks) * 1000 / RT_TICK_PER_SECOND);
    }

    rt_kprintf("CPU%d load thread exiting\n", ctrl->cpu_id);
    ctrl->thread = RT_NULL;
}

static void loadctl(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  loadctl stop               -- Stop all loads\n");
        rt_kprintf(
            "  loadctl [cpu_id] [load%%]   -- \
            Start/adjust load on specified CPU\n");
        return;
    }

    if (!strcmp(argv[1], "stop"))
    {
        for (int i = 0; i < MAX_CPUS; i++)
        {
            if (cpu_loads[i].thread)
            {
                cpu_loads[i].running = 0;
                rt_thread_delete(cpu_loads[i].thread);
                cpu_loads[i].thread = RT_NULL;
            }
        }
        rt_kprintf("All load threads stopped.\n");
    }
    else
    {
        int cpu = atoi(argv[1]);
        int load = (argc >= 3) ? atoi(argv[2]) : 50; /*default 50%*/

        if (cpu < 0 || cpu >= MAX_CPUS || load < 0 || load > 100)
        {
            rt_kprintf("Invalid cpu or load.\n");
            return;
        }

        cpu_load_control_t *ctrl = &cpu_loads[cpu];
        ctrl->cpu_id = cpu;
        ctrl->load_percent = load;

        if (ctrl->thread)
        {
            rt_kprintf("Adjusting CPU%d load to %d%%\n", cpu, load);
        }
        else
        {
            ctrl->running = 1;
            char name[RT_NAME_MAX];
            rt_snprintf(name, sizeof(name), "load%d", cpu);
            ctrl->thread = rt_thread_create(name, load_entry, ctrl,
                LOAD_THREAD_STACK_SIZE, LOAD_THREAD_PRIORITY, 10);
            if (ctrl->thread)
            {
                rt_thread_control(ctrl->thread, RT_THREAD_CTRL_BIND_CPU,
                    (void *)(rt_uint32_t)cpu);
                rt_thread_startup(ctrl->thread);
                rt_kprintf(
                    "Started load thread on CPU%d with %d%%\n", cpu, load);
            }
            else
            {
                rt_kprintf("Failed to create load thread for CPU%d\n", cpu);
            }
        }
    }
}
MSH_CMD_EXPORT(loadctl, Manage CPU load threads);

void load_perf_thread(void)
{
    loadctl(3, (char *[]){ "loadctl", "0", "80" });
    loadctl(3, (char *[]){ "loadctl", "1", "80" });
    loadctl(3, (char *[]){ "loadctl", "2", "80" });
    rt_perf(2, (char *[]){ "rt_perf", "thread_jitter" });
    rt_thread_mdelay(100);
    loadctl(2, (char *[]){ "loadctl", "stop" });
}
MSH_CMD_EXPORT(load_perf_thread, Load Thread Performance Test);

void load_perf_irq(void)
{
    loadctl(2, (char *[]){ "loadctl", "stop" });
    loadctl(3, (char *[]){ "loadctl", "0", "80" });
    loadctl(3, (char *[]){ "loadctl", "1", "80" });
    loadctl(3, (char *[]){ "loadctl", "2", "80" });
    rt_perf(2, (char *[]){ "rt_perf", "irq_latency" });
    rt_thread_mdelay(100);
    loadctl(2, (char *[]){ "loadctl", "stop" });
}
MSH_CMD_EXPORT(load_perf_irq, Load irq Performance Test);
