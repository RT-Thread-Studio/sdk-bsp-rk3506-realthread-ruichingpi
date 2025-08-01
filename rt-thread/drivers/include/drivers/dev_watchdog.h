/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DEV_WATCHDOG_H__
#define __DEV_WATCHDOG_H__

#include <rtthread.h>
#include <kpi.h>

#define RT_DEVICE_CTRL_WDT_GET_TIMEOUT                                         \
    (RT_DEVICE_CTRL_BASE(WDT) + 1) /* get timeout(in seconds) */
#define RT_DEVICE_CTRL_WDT_SET_TIMEOUT                                         \
    (RT_DEVICE_CTRL_BASE(WDT) + 2) /* set timeout(in seconds) */
#define RT_DEVICE_CTRL_WDT_GET_TIMELEFT                                        \
    (RT_DEVICE_CTRL_BASE(WDT) +                                                \
        3) /* get the left time before reboot(in seconds) */
#define RT_DEVICE_CTRL_WDT_KEEPALIVE                                           \
    (RT_DEVICE_CTRL_BASE(WDT) + 4) /* refresh watchdog */
#define RT_DEVICE_CTRL_WDT_START                                               \
    (RT_DEVICE_CTRL_BASE(WDT) + 5) /* start watchdog */
#define RT_DEVICE_CTRL_WDT_STOP                                                \
    (RT_DEVICE_CTRL_BASE(WDT) + 6) /* stop watchdog */

struct rt_watchdog_ops;
struct rt_watchdog_device
{
    struct rt_device parent;
    const struct rt_watchdog_ops *ops;
};
typedef struct rt_watchdog_device rt_watchdog_t;

struct rt_watchdog_ops
{
    rt_err_t (*init)(rt_watchdog_t *wdt);
    rt_err_t (*control)(rt_watchdog_t *wdt, int cmd, void *arg);
};

typedef rt_err_t (*__kpi_rt_hw_watchdog_register)(
    rt_watchdog_t *wdt, const char *name, rt_uint32_t flag, void *data);

KPI_EXTERN(rt_hw_watchdog_register);

#endif /* __DEV_WATCHDOG_H__ */
