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
#include <stdarg.h>

int fcntl(int fildes, int cmd, ...)
{
    va_list args;
    int result;

    va_start(args, cmd);

    result = rt_fcntl(fildes, cmd, args);

    va_end(args);

    return result;
}
