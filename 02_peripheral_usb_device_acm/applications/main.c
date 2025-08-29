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

int main(void)
{
    rt_kprintf("Hello, RT-Thread app\n");

    extern rt_err_t cdc_acm_example(void);
    cdc_acm_example();

    return 0;
}
