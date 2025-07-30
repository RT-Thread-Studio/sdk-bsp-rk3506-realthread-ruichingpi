/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <service.h>
#include <coredump_server.h>

struct service_core *cd_service;

extern const char __data_start[];
extern const char __data_end[];
extern const char __bss_cd_start[];
extern const char __bss_cd_end[];

rt_err_t coredump_init(void)
{
    struct app_coredump_table cd_table;

    cd_service = service_find("coredump_service");
    if (!cd_service)
    {
        rt_kprintf("service 'coredump_service' not found.\n");
        return (-RT_ERROR);
    }

    cd_table.data_start = (void *)__data_start;
    cd_table.data_end = (void *)__data_end;
    cd_table.bss_start = (void *)__bss_cd_start;
    cd_table.bss_end = (void *)__bss_cd_end;

    rt_err_t err = service_control(cd_service, APP_COREDUMP_TABLE_SET,
        &cd_table);
    if (err != RT_EOK)
    {
        rt_kprintf("failed to set coredump table.\n");
        return err;
    }

    return RT_EOK;
}
