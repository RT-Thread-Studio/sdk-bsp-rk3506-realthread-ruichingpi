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

static void level3_undefined_instruction(void)
{
    __asm__ __volatile__ (".word 0xE6000010");
}

static void level2_undefined_instruction(void)
{
    rt_kprintf("Entering level2_undefined_instruction\n");
    level3_undefined_instruction();
    rt_kprintf("This line should not be reached\n");
}

static void level1_undefined_instruction(void)
{
    rt_kprintf("Entering level1_undefined_instruction\n");
    level2_undefined_instruction();
    rt_kprintf("This line should not be reached\n");
}

static void undefined_instruction_thread_entry(void *parameter)
{
    rt_kprintf("Starting undefined instruction test\n");
    level1_undefined_instruction();
    rt_kprintf("This line should not be reached\n");
}

void trigger_undefined_instruction_exception(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("undef_test", undefined_instruction_thread_entry,
        RT_NULL, 1024, 20, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}
MSH_CMD_EXPORT(
    trigger_undefined_instruction_exception, Trigger undefined instruction exception test);
