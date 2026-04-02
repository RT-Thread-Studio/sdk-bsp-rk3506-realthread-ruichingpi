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
#include <rtdevice.h>

#define DBG_TAG "example.dsmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static int test_dsmc_read_write(void)
{
    int i;
    int err_count = 0;

    rt_uint32_t t_start, t_end;
    rt_uint32_t write_cycles, read_cycles;

    const int WORD_NUM = 4 * 1024;          // 4K word = 16KB
    uintptr_t BASE_ADDR = 0xC0000000;

    rt_uint32_t *write_buf = rt_malloc(WORD_NUM * 4);
    rt_uint32_t *read_buf  = rt_malloc(WORD_NUM * 4);

    if (!write_buf || !read_buf)
    {
        rt_kprintf("malloc failed!\n");
        return -1;
    }

    for (i = 0; i < WORD_NUM; i++)
    {
        write_buf[i] = 0x12345678 + i;
    }

    t_start = rt_hw_global_timer_get();

    for (i = 0; i < WORD_NUM; i++)
    {
        writel(write_buf[i], BASE_ADDR + i * 4);
    }

    t_end = rt_hw_global_timer_get();
    write_cycles = t_end - t_start;

    t_start = rt_hw_global_timer_get();

    for (i = 0; i < WORD_NUM; i++)
    {
        read_buf[i] = readl(BASE_ADDR + i * 4);
    }

    t_end = rt_hw_global_timer_get();
    read_cycles = t_end - t_start;

    for (i = 0; i < WORD_NUM; i++)
    {
        if (read_buf[i] != write_buf[i])
        {
            err_count++;
            rt_kprintf("ERR[%d]: R=0x%08X W=0x%08X\n",
                        i, read_buf[i], write_buf[i]);

            if (err_count > 10)
            {
                rt_kprintf("......\n");
                break;
            }
        }
    }

    rt_uint32_t write_time_us = write_cycles / 24;
    rt_uint32_t read_time_us  = read_cycles  / 24;

    rt_uint32_t total_bytes = WORD_NUM * 4;

    rt_uint32_t write_speed = (total_bytes / (1024)) / (write_time_us / 1e6);
    rt_uint32_t read_speed  = (total_bytes / (1024)) / (read_time_us  / 1e6);

    if (err_count == 0)
    {
        rt_kprintf("\n==== DSMC TEST PASS ====\n");

        rt_kprintf("Write: %lu cycles, %lu us, %lu KB/s\n",
                    write_cycles, write_time_us, write_speed);

        rt_kprintf("Read : %lu cycles, %lu us, %lu KB/s\n",
                    read_cycles, read_time_us, read_speed);
    }
    else
    {
        rt_kprintf("\n==== DSMC TEST FAIL ====\n");
        rt_kprintf("Error count: %d\n", err_count);
    }

    rt_free(write_buf);
    rt_free(read_buf);

    return 0;
}

MSH_CMD_EXPORT(test_dsmc_read_write, test dsmc read write);
