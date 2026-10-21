/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DISPLAY_SUBSYSTEM_H__
#define __DISPLAY_SUBSYSTEM_H__

#include <rtthread.h>
#include <kpi.h>

typedef rt_err_t (*__kpi_mipi_display_start)(void);
typedef rt_err_t (*__kpi_mipi_display_stop)(void);
typedef rt_err_t (*__kpi_rgb_display_start)(void);
typedef rt_err_t (*__kpi_rgb_display_stop)(void);

KPI_EXTERN(mipi_display_start);
KPI_EXTERN(mipi_display_stop);
KPI_EXTERN(rgb_display_start);
KPI_EXTERN(rgb_display_stop);

#endif /* __DISPLAY_SUBSYSTEM_H__ */
