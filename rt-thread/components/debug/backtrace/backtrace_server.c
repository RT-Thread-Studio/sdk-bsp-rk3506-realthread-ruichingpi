/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <backtrace_server.h>
#include <service.h>

struct service_core *bt_service;

extern const char __exidx_start[];
extern const char __exidx_end[];

rt_err_t backtrace_init(void)
{
    struct app_backtrace_table bt_table;

    bt_service = service_find("backtrace_service");
    if (!bt_service)
    {
        rt_kprintf("service 'backtrace_service' not found.\n");
        return (-RT_ERROR);
    }

    bt_table.backtrace_begin = (void *)__exidx_start;
    bt_table.backtrace_end = (void *)__exidx_end;

    rt_err_t err =
        service_control(bt_service, APP_BACKTRACE_TABLE_SET, &bt_table);
    if (err != RT_EOK)
    {
        rt_kprintf("failed to set backtrace table.\n");
        return err;
    }

    return RT_EOK;
}
