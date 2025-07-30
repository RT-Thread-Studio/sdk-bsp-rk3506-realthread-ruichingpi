/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */

#include <finsh_service.h>
#include <service.h>
#include <finsh.h>

struct service_core *service;

rt_err_t finsh_init(void)
{
    rt_err_t err;
    extern const int __app_fsymtab_start;
    extern const int __app_fsymtab_end;
    struct app_syscall_table syscall_table;

    service = service_find("finsh_service");
    if (!service)
    {
        rt_kprintf("not find finsh service\n");
        return (-RT_ERROR);
    }

    syscall_table._app_syscall_table_begin = (struct finsh_syscall *)&__app_fsymtab_start;
    syscall_table._app_syscall_table_end = (struct finsh_syscall *)&__app_fsymtab_end;
    err = service_control(service, APP_FINSH_TABLE_SET, &syscall_table);
    if (err != RT_EOK)
    {
        rt_kprintf("app syscall table set failed\n");
        return err;
    }

    return RT_EOK;
}

void finsh_set_echo(rt_uint32_t echo)
{
    rt_err_t ret;
    rt_uint32_t echo_value = echo;

    ret = service_control(service, APP_FINSH_SET_ECHO, (void*)&echo_value);
    if (ret != RT_EOK)
    {
        rt_kprintf("echo set: %d\n", ret);
    }
}

rt_uint32_t finsh_get_echo(void)
{
    rt_err_t ret;
    rt_uint32_t echo = 0;

    ret = service_control(service, APP_FINSH_GET_ECHO, (void*)&echo);
    if (ret != RT_EOK)
    {
        rt_kprintf("echo get failed: %d\n", ret);
        return 0;
    }

    return echo;
}

int msh_exec(char *cmd, rt_size_t length)
{
    rt_err_t ret;
    struct msh_cmd_args args;

    if (!cmd || length == 0)
    {
        rt_kprintf("[MSH] Invalid command (null or zero length)\n");
        return -1;
    }

    args.cmd = cmd;
    args.length = length;

    ret = service_control(service, APP_FINSH_MSH_EXEC, (void*)&args);
    if (ret != RT_EOK)
    {
        rt_kprintf("[MSH] Command execution failed: %d\n", ret);
        return -1;
    }

    return 0;
}

rt_device_t rt_console_set_device(const char *name)
{
    rt_err_t ret;
    struct finsh_console_device dev_info = {name, RT_NULL};

    /* Validate input */
    if (!name)
    {
        rt_kprintf("[CONSOLE] Error: NULL device name\n");
        return RT_NULL;
    }

    /* Call service control */
    ret = service_control(service, APP_FINSH_CONSOLE_SET_DEVICE, &dev_info);
    if (ret != RT_EOK)
    {
        rt_kprintf("[CONSOLE] Failed to set device: %d\n", ret);
        return RT_NULL;
    }

    return dev_info.old_dev;  /* Return old device handle */
}
