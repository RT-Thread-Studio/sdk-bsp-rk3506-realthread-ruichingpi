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
#include <fcntl.h>
#include <finsh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DBG_TAG "example.sdmmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define EXAMPLE_FILE    "/sdmmc/file_example.txt"
#define EXAMPLE_MESSAGE "This is an rt-thread sdmmc example"

static int sdmmc_example(void)
{
    int fd;
    rt_err_t rt_err = (-RT_ERROR);
    ssize_t read_size;
    ssize_t write_size;
    char buf[64];

    fd = open(EXAMPLE_FILE, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_E("Open file for write failed!");
        goto __exit;
    }

    write_size = write(fd, EXAMPLE_MESSAGE, strlen(EXAMPLE_MESSAGE));
    if (write_size < 0)
    {
        LOG_E("Write file failed!");
        goto __exit;
    }

    lseek(fd, 0, SEEK_SET);

    read_size = read(fd, buf, sizeof(buf) - 1);
    if (read_size < 0)
    {
        rt_kprintf("Read file failed!\n");
        goto __exit;
    }

    if (read_size != write_size)
    {
        LOG_E("read failed, read size: %d, write size: %d", read_size,
            write_size);

        goto __exit;
    }
    buf[read_size] = '\0';

    LOG_I("read content: %s", buf);

    rt_err = RT_EOK;

__exit:
    close(fd);

    return rt_err;
}
MSH_CMD_EXPORT(sdmmc_example, sdmmc example);
