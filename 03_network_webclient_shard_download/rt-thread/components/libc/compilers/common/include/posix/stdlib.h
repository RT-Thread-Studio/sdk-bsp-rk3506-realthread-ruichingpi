/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __POSIX_STDLIB_H__
#define __POSIX_STDLIB_H__

#include <stdint.h>
#include <stdlib.h>
#include <kpi.h>

typedef void (*__kpi___rt_libc_exit)(int status);

char *itoa(int n, char *buffer, int radix);
char *lltoa(int64_t ll, char *buffer, int radix);
char *ltoa(long l, char *buffer, int radix);
char *ulltoa(uint64_t ll, char *buffer, int radix);
char *ultoa(unsigned long l, char *buffer, int radix);
char *utoa(unsigned int n, char *buffer, int radix);

KPI_EXTERN(__rt_libc_exit);

#endif /* __POSIX_STDLIB_H__ */
