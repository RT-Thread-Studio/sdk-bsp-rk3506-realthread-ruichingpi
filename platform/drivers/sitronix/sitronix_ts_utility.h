/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SITRONIX_TS_UTILITY_H__
#define __SITRONIX_TS_UTILITY_H__

#include <sitronix_ts.h>

int sitronix_ts_get_device_status(struct sitronix_ts_data *ts_data);
int sitronix_ts_get_device_info(struct sitronix_ts_data *ts_data);
void st_checksum_calculation(
    unsigned short *pChecksum, unsigned char *pInData, unsigned long Len);

#endif /* __SITRONIX_TS_UTILITY_H__ */
