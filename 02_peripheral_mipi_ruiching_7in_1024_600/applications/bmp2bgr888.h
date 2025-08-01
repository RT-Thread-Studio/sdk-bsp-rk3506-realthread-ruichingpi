/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __BMP2BGR888_H__
#define __BMP2BGR888_H__

#include <rtthread.h>

rt_err_t bmp_extract_bgr888(const rt_uint8_t *bmp_data, rt_uint8_t *img_buffer);

#endif /* __BMP2RGB888_H__ */
