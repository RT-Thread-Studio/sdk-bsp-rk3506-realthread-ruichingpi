/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __CPU_USAGE_H__
#define __CPU_USAGE_H__

#include <rtthread.h>

/**
 * @brief Get all CPU usage values from one sampling window.
 *
 * @param[in]  sample_ms Sampling time from 10 to 10000 milliseconds.
 * @param[out] usage     Array of RT_CPUS_NR percentages in the range 0 to 100.
 *
 * @note This function blocks and must run in thread context. The first call
 *       installs the system's single tick hook.
 *
 * @return RT_EOK on success, -RT_EINVAL for invalid arguments, or
 *         -RT_ETIMEOUT if a CPU produces no tick during the window.
 */
rt_err_t get_cpu_usage(rt_uint32_t sample_ms,
    rt_uint8_t usage[RT_CPUS_NR]);

#endif /* __CPU_USAGE_H__ */
