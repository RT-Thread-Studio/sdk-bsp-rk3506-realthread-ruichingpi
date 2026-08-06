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
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dfs_fs.h>
#include "dfs_tmpfs.h"
#define DBG_TAG "example.tmpfs"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define MOUNT_PATH  "/data/tmpfs"
#define SUB_DIR     MOUNT_PATH "/subdir"
#define TEST_FILE   SUB_DIR "/test.txt"
#define TEST_DATA   "Hello RT-Thread TMPFS!"

int tmpfs_register_example(void)
{
    rt_err_t ret = RT_EOK;
    int fd;
    ssize_t write_size;
    ssize_t read_size;
    char buf[64];

    if (dfs_tmpfs_init() != 0)
    {
        LOG_E("tmpfs register failed");
        ret = -RT_ERROR;
        goto __exit;
    }
    LOG_I("tmpfs filesystem registered");

    mkdir(MOUNT_PATH, 0);

    if (dfs_mount(RT_NULL, MOUNT_PATH, "tmp", 0, RT_NULL) != 0)
    {
        LOG_E("tmpfs mount failed");
        ret = -RT_ERROR;
        goto __exit;
    }
    LOG_I("tmpfs mounted at %s", MOUNT_PATH);

    mkdir(SUB_DIR, 0);

    fd = open(TEST_FILE, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_E("open file failed");
        ret = -RT_ERROR;
        goto __exit;
    }

    write_size = write(fd, TEST_DATA, strlen(TEST_DATA));
    if (write_size < 0)
    {
        LOG_E("write file failed");
        close(fd);
        ret = -RT_ERROR;
        goto __exit;
    }

    lseek(fd, 0, SEEK_SET);

    read_size = read(fd, buf, sizeof(buf) - 1);
    if (read_size < 0)
    {
        LOG_E("read file failed");
        close(fd);
        ret = -RT_ERROR;
        goto __exit;
    }
    buf[read_size] = '\0';

    LOG_I("write %d bytes, read back: %s", write_size, buf);

    close(fd);

__exit:
    unlink(TEST_FILE);
    LOG_I("cleanup: unlink %s", TEST_FILE);
    unlink(SUB_DIR);
    LOG_I("cleanup: unlink %s", SUB_DIR);

    if (dfs_unmount(MOUNT_PATH) == 0)
        LOG_I("cleanup: unmount %s OK", MOUNT_PATH);
    else
        LOG_I("cleanup: unmount %s skipped (not mounted)", MOUNT_PATH);

    unlink(MOUNT_PATH);

    return ret;
}
MSH_CMD_EXPORT(tmpfs_register_example, tmpfs register and rw example);
