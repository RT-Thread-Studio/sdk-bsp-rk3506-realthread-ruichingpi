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
#include <unistd.h>
#define DBG_TAG "example.rmdir"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define EXAMPLE_DIR "/data/example_dir"

static int rmdir_example(void)
{
    rt_err_t ret;

    ret = unlink(EXAMPLE_DIR);
    if (ret)
    {
        LOG_E("The deletion of the \"example_dir\" folder failed.");
        goto __exit;
    }

    LOG_I("The deletion of the \"example_dir\" folder was successful.");

__exit:

    return ret;
}
MSH_CMD_EXPORT(rmdir_example, rmdir example);
