/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __IO_H__
#define __IO_H__

#include <rttypes.h>

#ifndef readb
#define readb(reg)              (*((volatile rt_uint8_t *)(reg)))
#endif /* readb */

#ifndef readw
#define readw(reg)              (*((volatile rt_uint16_t *)(reg)))
#endif /* readw */

#ifndef readl
#define readl(reg)              (*((volatile rt_uint32_t *)(reg)))
#endif /* readl */

#ifndef readq
#define readq(reg)              (*((volatile rt_uint64_t *)(reg)))
#endif /* readq */

#ifndef writeb
#define writeb(data, reg)       ((*((volatile rt_uint8_t *)(reg))) = (rt_uint8_t)(data))
#endif /* writeb */

#ifndef writew
#define writew(data, reg)       ((*((volatile rt_uint16_t *)(reg))) = (rt_uint16_t)(data))
#endif /* writew */

#ifndef writel
#define writel(data, reg)       ((*((volatile rt_uint32_t *)(reg))) = (rt_uint32_t)(data))
#endif /* writel */

#ifndef writeq
#define writeq(data, reg)       ((*((volatile rt_uint64_t *)(reg))) = (rt_uint64_t)(data))
#endif /* writeq */

#endif /* __IO_H__ */