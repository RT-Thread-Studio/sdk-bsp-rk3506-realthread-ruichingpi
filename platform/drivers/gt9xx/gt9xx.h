/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __GT9xx_H__
#define __GT9xx_H__

#include <rtthread.h>
#include <rtdevice.h>

#define GT_DEBUG          0

#define gt_err(fmt, ...)  rt_kprintf("ERR: " fmt, ##__VA_ARGS__)
#define gt_warn(fmt, ...) rt_kprintf("WARN: " fmt, ##__VA_ARGS__)

#if (GT_DEBUG)
#define gt_dbg(fmt, ...) rt_kprintf("DBG: " fmt, ##__VA_ARGS__)
#else
#define gt_dbg(fmt, ...)
#endif /* GT_DEBUG */

/* Registers define */
#define GTP_READ_COOR_ADDR    0x814E
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814A
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8140

#define GTP_REG_POINT1        0x814F

#define GTP_ADDR_LENGTH       2
#define GTP_CONFIG_MIN_LENGTH 186
#define GTP_CONFIG_MAX_LENGTH 240

#define RESOLUTION_LOC        3
#define TRIGGER_LOC           8

#define GTP_MAX_TOUCH         5
#define GTP_POINT_INFO_NUM    8

#endif /* __GT9xx_H__ */
