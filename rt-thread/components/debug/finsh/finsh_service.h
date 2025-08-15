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

#ifndef __FINSH_SERVICE_H__
#define __FINSH_SERVICE_H__

#define APP_FINSH_TABLE_SET          0x01
#define APP_FINSH_SET_ECHO           0x02
#define APP_FINSH_GET_ECHO           0x03
#define APP_FINSH_MSH_EXEC           0x04
#define APP_FINSH_CONSOLE_SET_DEVICE 0x05
#define APP_FINSH_CONSOLE_GET_DEVICE 0x06
#define APP_FINSH_GET_PROMPT         0x07

struct app_syscall_table
{
    struct finsh_syscall *_app_syscall_table_begin;
    struct finsh_syscall *_app_syscall_table_end;
};

struct msh_cmd_args
{
    char *cmd;
    rt_size_t length;
};

struct finsh_console_device
{
    const char *name;
    rt_device_t cur_dev;
    rt_device_t old_dev;
};

rt_err_t finsh_init(void);

struct service_core *finsh_service_get(void);

rt_device_t rt_console_set_device(const char *name);

rt_device_t rt_console_get_device(void);

#endif /* __FINSH_SERVICE_H__ */
