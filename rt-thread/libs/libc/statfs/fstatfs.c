/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <newlib_compiler.h>
#include <sys/statfs.h>

int fstatfs(int fd, struct statfs *buf)
{
    return rt_fstatfs(fd, buf);
}

int fstat(int fildes, struct stat *buf)
{
    return rt_fstat(fildes, buf);
}
