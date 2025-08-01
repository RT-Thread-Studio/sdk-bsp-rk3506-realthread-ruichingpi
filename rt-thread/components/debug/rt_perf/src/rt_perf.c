/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "rt_perf.h"
#include <rtthread.h>
#include "rt_perf_testcase.h"

#ifdef RT_PERF_EXPORT_CSV_DATA
#include <dfs_posix.h>
#endif /* RT_PERF_EXPORT_CSV_DATA */

#define PERF_UP           1
#define PERF_DOWN         0

#define RET_INT           0
#define RET_DECIMALS      1
#define GET_INT(num)      split_double(num, RET_INT)
#define GET_DECIMALS(num) split_double(num, RET_DECIMALS)

static char *testcase_name[] = {
    "thread_jitter",
#if defined(RT_PERF_ENABLE_IRQ_LATENCY)
    "irq_latency",
#endif /* RT_PERF_ENABLE_IRQ_LATENCY */
};

typedef rt_err_t (*testcase_function)(struct rt_perf *perf);

testcase_function test_func_ptrs[] = {
    rt_perf_thread_jitter,
#if defined(RT_PERF_ENABLE_IRQ_LATENCY)
    rt_perf_irq_latency,
#endif /* RT_PERF_ENABLE_IRQ_LATENCY */
};

rt_weak rt_uint32_t rt_hw_global_timer_get_tick(void)
{
    return rt_hw_global_timer_get();
}

static rt_int32_t split_double(double num, rt_uint32_t type)
{
    if (type == RET_INT)
    {
        return (rt_int32_t)num;
    }
    else if (type == RET_DECIMALS)
    {
        return (rt_int32_t)((num - (rt_int32_t)num) * 10000);
    }
    else
    {
        return (-1);
    }

    return (-1);
}

static struct rt_perf *rt_perf_malloc_mem(void)
{
    struct rt_perf *perf = RT_NULL;

    perf = rt_malloc_align(sizeof(struct rt_perf), RT_ALIGN_SIZE);
    if (perf == RT_NULL)
    {
        return RT_NULL;
    }

#ifdef RT_PERF_EXPORT_CSV_DATA
    perf->value =
        (rt_uint32_t *)rt_malloc_align((PERF_COUNT * 4), RT_ALIGN_SIZE);
    if (perf->value == RT_NULL)
    {
        return RT_NULL;
    }
#endif /* RT_PERF_EXPORT_CSV_DATA */

    return perf;
}

#ifdef RT_PERF_EXPORT_CSV_DATA
extern void copy(const char *src, const char *dst);

static void export_csv_data(struct rt_perf *perf)
{
    char path[64] = { 0 };
    char tmp_path[64] = { 0 };
    char ch = 0;
    char buf[16] = { 0 };
    rt_int32_t fd;
    rt_int32_t tmp_fd;
    rt_int32_t index = 0;
    double timer = 0.0f;

    rt_sprintf(path, "%s/%s", RT_PERF_EXPORT_CSV_DATA_PATH, EXPORT_DATA_NAME);
    rt_sprintf(
        tmp_path, "%s/tmp_%s", RT_PERF_EXPORT_CSV_DATA_PATH, EXPORT_DATA_NAME);

    if (perf->info_head)
    {
        fd = open(path, O_RDWR);
        if (fd)
        {
            close(fd);
            remove(path);
        }
        fd = open(path, O_CREAT | O_RDWR);

        rt_sprintf(buf, "%s\n", perf->name);

        for (index = 0; index < perf->count; index++)
        {
            if (index == 0)
            {
                rt_sprintf(buf, "%s\n", perf->name);
            }
            else
            {
                timer =
                    ((double)perf->value[index] / perf->timer_frq) * 1000000;
                rt_sprintf(
                    buf, "%u.%04u\n", GET_INT(timer), GET_DECIMALS(timer));
            }
            write(fd, buf, rt_strlen(buf));
        }
        close(fd);
        return;
    }
    else
    {
        tmp_fd = open(tmp_path, O_CREAT | O_RDWR);
        fd = open(path, O_RDWR);
    }
    lseek(fd, 0, SEEK_SET);

    while (read(fd, &ch, 1) > 0)
    {
        if (ch == '\n')
        {
            if (index == 0)
            {
                rt_sprintf(buf, ",%s\n", perf->name);
            }
            else
            {
                timer =
                    ((double)perf->value[index] / perf->timer_frq) * 1000000;
                rt_sprintf(
                    buf, ",%u.%04u\n", GET_INT(timer), GET_DECIMALS(timer));
            }
            write(tmp_fd, buf, rt_strlen(buf));
            index++;
        }
        else
        {
            write(tmp_fd, &ch, 1);
        }
    }
    close(tmp_fd);
    close(fd);

    remove(path);
    copy(tmp_path, path);
    remove(tmp_path);
}
#endif /* RT_PERF_EXPORT_CSV_DATA */

void rt_perf_start(struct rt_perf *perf)
{
    perf->begin_time = rt_hw_global_timer_get();
}

void rt_perf_stop(struct rt_perf *perf)
{
    if (perf->timer_dir)
    {
        perf->real_time = rt_hw_global_timer_get() - perf->begin_time;
    }
    else
    {
        perf->real_time = perf->begin_time - rt_hw_global_timer_get();
    }

    if (perf->manual_change)
    {
        perf->manual_change(perf);
    }

    if ((perf->real_time >> (RT_PERF_TIMER_BITS - 1)))
    {
        return;
    }

    if (perf->real_time > perf->max_time)
    {
        perf->max_time = perf->real_time;
    }

    if (perf->real_time < perf->min_time)
    {
        perf->min_time = perf->real_time;
    }

    perf->tot_time += perf->real_time;
    perf->count++;

#ifdef RT_PERF_EXPORT_CSV_DATA
    perf->value[perf->count] = perf->real_time;
#endif /* RT_PERF_EXPORT_CSV_DATA */
}

void rt_perf_dump(struct rt_perf *perf)
{
    char min_str[10] = { 0 };
    char max_str[10] = { 0 };
    char avg_str[10] = { 0 };
    rt_uint32_t total = 0;
    double speed = 0.0f;
    double min = 0.0f;
    double max = 0.0f;
    double avg = 0.0f;

    if ((perf->timer_frq == 0) || (perf->count == 0))
    {
        rt_kprintf(
            "perf timer frequency is 0, please check the timer frequency.\n");
        return;
    }

    total = (rt_uint32_t)(perf->tot_time / (perf->timer_frq / 1000000));
    perf->avg_time = perf->tot_time / perf->count;

    /* print info head */
    if (perf->info_head)
    {
        rt_kprintf(
            "perf               count     total(us)  min(us)  max(us)  avg(us)  speed(MB/s)\n");
        rt_kprintf(
            "------------------ --------- ---------- -------- -------- -------- ---------------\n");
    }

    /* print data info */
    if (rt_strstr(perf->name, "-read") || rt_strstr(perf->name, "-write"))
    {
        speed = (KB2MB(MEMORY_SIZE * MEM_LOOP) / (total / 1000000.0f));
        rt_kprintf("%-18s %-9d %-10d %-8s %-8s %-8s %-d.%04d\n", perf->name,
            MEM_LOOP, total, "-", "-", "-", GET_INT(speed),
            GET_DECIMALS(speed));
    }
    else if (!rt_strcmp(perf->name, "float"))
    {
        rt_kprintf("%-18s %-9d %-10d %-8s %-8s %-8s %-15s\n", perf->name,
            perf->count, perf->tot_time, "-", "-", "-", "-");
    }
    else
    {
        min = ((double)perf->min_time / (perf->timer_frq / 1000000));
        max = ((double)perf->max_time / (perf->timer_frq / 1000000));
        avg = ((double)perf->avg_time / (perf->timer_frq / 1000000));

        rt_sprintf(min_str, "%u.%04u", GET_INT(min), GET_DECIMALS(min));
        rt_sprintf(max_str, "%u.%04u", GET_INT(max), GET_DECIMALS(max));
        rt_sprintf(avg_str, "%u.%04u", GET_INT(avg), GET_DECIMALS(avg));

        rt_kprintf("%-18s %-9u %-10u %-8s %-8s %-8s %-15s\n", perf->name,
            perf->count, total, min_str, max_str, avg_str, "-");

#ifdef RT_PERF_EXPORT_CSV_DATA
        export_csv_data(perf);
#endif /* RT_PERF_EXPORT_CSV_DATA */
    }
}

void rt_perf_clear(struct rt_perf *perf)
{
    perf->info_head = RT_TRUE;
    perf->timer_frq = RT_PERF_TIMER_FREQ;
    perf->begin_time = 0;
    perf->real_time = 0;
    perf->tot_time = 0;
    perf->max_time = 0;
    perf->min_time = RT_UINT32_MAX;
    perf->avg_time = 0;
    perf->count = 0;
    perf->manual_change = RT_NULL;

#ifdef RT_PERF_EXPORT_CSV_DATA
    if (perf->value)
    {
        rt_memset(perf->value, 0x00, (PERF_COUNT * 4));
    }
#endif /* RT_PERF_EXPORT_CSV_DATA */
}

void rt_perf_manual_set(
    struct rt_perf *perf, void (*manual_change)(struct rt_perf *perf))
{
    if (manual_change != RT_NULL)
    {
        perf->manual_change = manual_change;
    }
}

#ifdef FINSH_OPTION_COMPLETION_ENABLED
#include <msh.h>
CMD_OPTIONS_STATEMENT(rt_perf);
#endif /* FINSH_OPTION_COMPLETION_ENABLED */
int rt_perf(int argc, char *argv[])
{
    rt_int32_t start_time = 0;
    rt_int32_t end_time = 0;
    rt_uint32_t testcase_num;
    static struct rt_perf *perf = RT_NULL;

    perf = rt_perf_malloc_mem();
    if (perf == RT_NULL)
    {
        rt_kprintf("malloc mem failed.\n");
        return (-RT_ENOMEM);
    }

    rt_perf_clear(perf);

    start_time = rt_hw_global_timer_get();
    rt_thread_mdelay(1);
    end_time = rt_hw_global_timer_get();
    if (start_time == end_time)
    {
        rt_kprintf("global timer not working\n");
        return (-RT_ERROR);
    }
    else
    {
        (start_time < end_time) ? (perf->timer_dir = PERF_UP) :
                                  (perf->timer_dir = PERF_DOWN);
    }

    if (argc == 2)
    {
        testcase_num = sizeof(testcase_name) / sizeof((testcase_name)[0]);

        if (!rt_strcmp(argv[1], "all"))
        {
            for (rt_uint32_t i = 0; i < testcase_num; i++)
            {
                rt_memset(perf->name, 0x00, 32);
                rt_memcpy(
                    perf->name, testcase_name[i], rt_strlen(testcase_name[i]));

                test_func_ptrs[i](perf);

                rt_thread_mdelay(100);

                rt_perf_clear(perf);
                perf->info_head = RT_FALSE;
            }
        }
        else
        {
            for (rt_uint32_t i = 0; i < testcase_num; i++)
            {
                rt_memset(perf->name, 0x00, 32);
                rt_memcpy(perf->name, argv[1], rt_strlen(argv[1]));

                if (!rt_strcmp(perf->name, testcase_name[i]))
                {
                    test_func_ptrs[i](perf);
                    break;
                }

                if (i == (testcase_num - 1))
                {
                    rt_kprintf("not find testcase(%s)\n", argv[1]);
                }
            }
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("rt_perf [tasecase]\n\n");
        rt_kprintf("tasecase:\n");
        rt_kprintf("  thread_jitter         - thread jitter time\n");
#if defined(RT_PERF_ENABLE_IRQ_LATENCY)
        rt_kprintf("  irq_latency           - irq latency\n");
#endif /* RT_PERF_ENABLE_IRQ_LATENCY */
    }

#ifdef RT_PERF_EXPORT_CSV_DATA
    rt_free_align(perf->value);
#endif /* RT_PERF_EXPORT_CSV_DATA */
    rt_free_align(perf);

    return RT_EOK;
}
#ifdef FINSH_USING_OPTION_COMPLETION
CMD_OPTIONS_NODE_START(rt_perf)
CMD_OPTIONS_NODE(1, thread_jitter, thread jitter time)
#if defined(RT_PERF_ENABLE_IRQ_LATENCY)
CMD_OPTIONS_NODE(2, irq_latency, irq latency)
#endif /* RT_PERF_ENABLE_IRQ_LATENCY */
CMD_OPTIONS_NODE_END
MSH_CMD_EXPORT(rt_perf, performance, optenable);
#else
MSH_CMD_EXPORT(rt_perf, performance);
#endif /* FINSH_USING_OPTION_COMPLETION */
