/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <sys/errno.h>
#include <newlib_compiler.h>
#include <dirent.h>

int closedir(DIR *d)
{
    return rt_closedir(d);
}
