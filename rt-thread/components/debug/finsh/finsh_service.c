/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <finsh.h>
#include <finsh_service.h>
#include <service.h>

struct service_core *gs_finsh_service = RT_NULL;

rt_err_t finsh_init(void)
{
    rt_err_t err;
    extern const int __app_fsymtab_start;
    extern const int __app_fsymtab_end;
    struct app_syscall_table syscall_table;

    gs_finsh_service = service_find("finsh_service");
    if (!gs_finsh_service)
    {
        rt_kprintf("not find finsh service\n");
        return (-RT_ERROR);
    }

    syscall_table._app_syscall_table_begin =
        (struct finsh_syscall *)&__app_fsymtab_start;
    syscall_table._app_syscall_table_end =
        (struct finsh_syscall *)&__app_fsymtab_end;
    err =
        service_control(gs_finsh_service, APP_FINSH_TABLE_SET, &syscall_table);
    if (err != RT_EOK)
    {
        rt_kprintf("app syscall table set failed\n");
        return err;
    }

    return RT_EOK;
}

struct service_core *finsh_service_get(void)
{
    return gs_finsh_service;
}

rt_device_t rt_console_set_device(const char *name)
{
    rt_err_t ret;
    struct finsh_console_device dev_info = { name, RT_NULL, RT_NULL };

    /* Validate input */
    if (!name)
    {
        rt_kprintf("[CONSOLE] Error: NULL device name\n");
        return RT_NULL;
    }

    /* Call service control */
    ret = service_control(
        gs_finsh_service, APP_FINSH_CONSOLE_SET_DEVICE, &dev_info);
    if (ret != RT_EOK)
    {
        rt_kprintf("[CONSOLE] Failed to set device: %d\n", ret);
        return RT_NULL;
    }

    return dev_info.old_dev; /* Return old device handle */
}

rt_device_t rt_console_get_device(void)
{
    rt_err_t ret;
    struct finsh_console_device dev_info = { RT_NULL, RT_NULL, RT_NULL };

    /* Call service control */
    ret = service_control(
        gs_finsh_service, APP_FINSH_CONSOLE_GET_DEVICE, &dev_info);
    if (ret != RT_EOK)
    {
        rt_kprintf("[CONSOLE] Failed to set device: %d\n", ret);
        return RT_NULL;
    }

    return dev_info.cur_dev;
}
