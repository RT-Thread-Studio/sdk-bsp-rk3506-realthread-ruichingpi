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
extern int lvgl_thread_init(void);

int main(void)
{
    rt_kprintf("Hello, RT-Thread app\n");
    lvgl_thread_init();

    return 0;
}
