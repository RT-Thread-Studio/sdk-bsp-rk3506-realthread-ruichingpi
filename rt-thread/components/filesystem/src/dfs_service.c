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
#include <dfs_service.h>

static struct service_core *dfs_service = RT_NULL;

struct dfs_fdtable *dfs_fdtable_get(void)
{
    rt_err_t ret;
    struct dfs_fdtable *fdt;

    ret = service_control(dfs_service, APP_DFS_FDTABLE_GET, &fdt);

    if (ret!= RT_EOK)
    {
        return RT_NULL;
    }

    return fdt;
}
