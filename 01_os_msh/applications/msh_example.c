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
#include <rtdevice.h>

#define DBG_TAG "example.msh"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void msh_example(int argc, char **argv)
{
    const char *cmd = argv[1];

    if (argc < 2)
    {
        LOG_E("Error: missing arguments.");
        LOG_I("Usage: msh_example [test1|test2|option value|help]");
        return;
    }

    if (rt_strcmp(cmd, "test1") == 0)
    {
        LOG_I("Executing Test 1...");
    }
    else if (rt_strcmp(cmd, "test2") == 0)
    {
        LOG_I("Executing Test 2...");
    }
    else if (rt_strcmp(cmd, "option") == 0)
    {
        if (argc < 3)
        {
            LOG_E("Error: option requires a value.");
            return;
        }
        LOG_I("Option value: %s\n", argv[2]);
    }
    else if (rt_strcmp(cmd, "help") == 0)
    {
        LOG_I("Help:");
        LOG_I("  msh_example test1          - Execute test 1");
        LOG_I("  msh_example test2          - Execute test 2");
        LOG_I("  msh_example option [value] - Print option value");
        LOG_I("  msh_example help           - Show help");
    }
    else
    {
        LOG_E("Error: unknown command '%s'.\n", cmd);
        LOG_I("Usage: msh_example [test1|test2|option value|help]");
    }
}
MSH_CMD_EXPORT(msh_example, MSH test command);
