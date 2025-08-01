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
#include <msh.h>
#include <service.h>

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

    ret =
        service_control(finsh_service_get(), APP_FINSH_MSH_EXEC, (void *)&args);
    if (ret != RT_EOK)
    {
        rt_kprintf("[MSH] Command execution failed: %d\n", ret);
        return -1;
    }

    return 0;
}
