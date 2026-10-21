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
#include "display_subsystem.h"

extern int lvgl_thread_init(void);
extern int ethercat_domain_init(void);

int main(void)
{
    rt_kprintf("Hello, RT-Thread app\n");

    if (mipi_display_start() != RT_EOK)
    {
        rt_kprintf("mipi display start failed\n");
    }

    lvgl_thread_init();
    ethercat_domain_init();

    return 0;
}
