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

int statfs(const char *path, struct statfs *buf)
{
    return rt_statfs(path, buf);
}
