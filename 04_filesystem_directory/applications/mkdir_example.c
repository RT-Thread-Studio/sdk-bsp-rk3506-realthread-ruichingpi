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
#include <finsh.h>
#include <sys/stat.h>
#include <sys/types.h>
#define DBG_TAG "example.mkdir"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define EXAMPLE_DIR "/data/example_dir"

static int mkdir_example(void)
{
    rt_err_t ret;

    ret = mkdir(EXAMPLE_DIR, 0);
    if (ret)
    {
        LOG_E("Failed to create the \"example_dir\" folder.");
        goto __exit;
    }

    LOG_I("The \"example_dir\" folder was created successfully.");

__exit:

    return ret;
}
MSH_CMD_EXPORT(mkdir_example, mkdir example);
