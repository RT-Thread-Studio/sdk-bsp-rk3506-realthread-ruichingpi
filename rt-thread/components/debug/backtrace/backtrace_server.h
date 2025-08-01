/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __BACKTRACE_SERVER_H__
#define __BACKTRACE_SERVER_H__

#include <rtthread.h>

#define APP_BACKTRACE_TABLE_SET 0x01

struct app_backtrace_table
{
    void *backtrace_begin;
    void *backtrace_end;
};

rt_err_t backtrace_init(void);

#endif /* __BACKTRACE_SERVER_H__ */
