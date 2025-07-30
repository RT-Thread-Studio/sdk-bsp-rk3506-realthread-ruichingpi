/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __KSTDIO_H__
#define __KSTDIO_H__

#include <rttypes.h>
#include <stdarg.h>
#include <kpi.h>

typedef int (*__kpi_rt_vsprintf)(char *dest, const char *format, va_list arg_ptr);
typedef int (*__kpi_rt_vsnprintf)(char *buf, rt_size_t size, const char *fmt, va_list args);
typedef int (*__kpi_rt_sprintf)(char *buf, const char *format, ...);
typedef int (*__kpi_rt_snprintf)(char *buf, rt_size_t size, const char *format, ...);
typedef int (*__kpi_rt_vsscanf)(const char *buffer, const char *format, va_list ap);
typedef int (*__kpi_rt_sscanf)(const char *str, const char *format, ...);

KPI_EXTERN(rt_vsprintf);
KPI_EXTERN(rt_vsnprintf);
KPI_EXTERN(rt_sprintf);
KPI_EXTERN(rt_snprintf);
KPI_EXTERN(rt_vsscanf);
KPI_EXTERN(rt_sscanf);

#endif /* __KSTDIO_H__ */
