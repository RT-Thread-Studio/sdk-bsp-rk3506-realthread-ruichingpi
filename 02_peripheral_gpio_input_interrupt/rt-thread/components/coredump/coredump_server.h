/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __COREDUMP_SERVER_H__
#define __COREDUMP_SERVER_H__

#include <rtthread.h>

#define APP_COREDUMP_TABLE_SET 0x01

struct app_coredump_table
{
    void *data_start;
    void *data_end;
    void *bss_start;
    void *bss_end;
};

rt_err_t coredump_init(void);

#endif /* __COREDUMP_SERVER_H__ */
