/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <rtthread.h>
#include <rtdevice.h>

#define TIMER_LOAD_COUNT0    0x00
#define TIMER_LOAD_COUNT1    0x04
#define TIMER_CURRENT_VALUE0 0x08
#define TIMER_CURRENT_VALUE1 0x0c

#define TIMER_INTSTATUS      0x18
#define INTSTATUS_EN         RT_BIT(0)

#define TIMER_CONTROLREG     0x10
#define CONTROL_TIMER_EN     RT_BIT(0)
#define CONTROL_TIMER_MODE   RT_BIT(1)
#define CONTROL_TIMER_INT_EN RT_BIT(2)

#endif /* __DRV_HWTIMER_H__ */