/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __KPI_H__
#define __KPI_H__

#include <rttypes.h>

#define KPI_DEFINED(symbol) __kpi_##symbol symbol
#define KPI_EXTERN(symbol)  extern __kpi_##symbol symbol
#define KPI_IMPORT(symbol, index)                                              \
    (__kpi_##symbol)(                                                          \
        *(rt_ubase_t *)(__kpi_start + ((index - 1) * sizeof(rt_ubase_t))));

void kpi_init(void);

#endif /* __KPI_H__ */
