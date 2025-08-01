/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __KSTRING_H__
#define __KSTRING_H__

#include <rttypes.h>
#include <kpi.h>

typedef void *(*__kpi_rt_memset)(void *src, int c, rt_ubase_t n);
typedef void *(*__kpi_rt_memcpy)(void *dest, const void *src, rt_ubase_t n);
typedef void *(*__kpi_rt_memmove)(void *dest, const void *src, rt_size_t n);
typedef rt_int32_t (*__kpi_rt_memcmp)(const void *cs, const void *ct,
    rt_size_t count);

typedef char *(*__kpi_rt_strdup)(const char *s);
typedef rt_size_t (*__kpi_rt_strnlen)(const char *s, rt_ubase_t maxlen);
typedef char *(*__kpi_rt_strstr)(const char *str1, const char *str2);
typedef rt_int32_t (*__kpi_rt_strcasecmp)(const char *a, const char *b);
typedef char *(*__kpi_rt_strcpy)(char *dst, const char *src);
typedef char *(*__kpi_rt_strncpy)(char *dest, const char *src, rt_size_t n);
typedef rt_int32_t (*__kpi_rt_strncmp)(const char *cs, const char *ct,
    rt_size_t count);
typedef rt_int32_t (*__kpi_rt_strcmp)(const char *cs, const char *ct);
typedef rt_size_t (*__kpi_rt_strlen)(const char *src);

KPI_EXTERN(rt_memset);
KPI_EXTERN(rt_memcpy);
KPI_EXTERN(rt_memmove);
KPI_EXTERN(rt_memcmp);
KPI_EXTERN(rt_strdup);
KPI_EXTERN(rt_strnlen);
KPI_EXTERN(rt_strstr);
KPI_EXTERN(rt_strcasecmp);
KPI_EXTERN(rt_strcpy);
KPI_EXTERN(rt_strncpy);
KPI_EXTERN(rt_strncmp);
KPI_EXTERN(rt_strcmp);
KPI_EXTERN(rt_strlen);

#endif /* __KSTRING_H__ */
