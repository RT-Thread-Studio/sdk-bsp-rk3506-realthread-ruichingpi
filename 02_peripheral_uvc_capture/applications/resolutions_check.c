/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */


#include <stdio.h>
#include <uvc_buffer.h>
// 定义 jpg 格式支持的分辨率
struct rt_uvc_resolution mjpeg_resolutions[] = {
    {640, 480},
    {640, 360},
    {352, 288},
    {320, 240},
    {1280, 720},
    {1600, 1200},
    {1920, 1080},
    {2048, 1536},
    {2560, 1440},
    {2592, 1944},
};

// 定义 YUV 格式支持的分辨率
struct rt_uvc_resolution yuv_resolutions[] = {
    {1280, 720},
    {640, 480},
    {320, 240},
    {1600, 1200},
    {1920, 1080},
    {2048, 1536},
    {2560, 1440},
    {2592, 1944},
};

// 检查 MJPEG 格式的分辨率是否有效
int mjpeg_resolution_supported(uint16_t width, uint16_t height) {
    for (int i = 0; i < sizeof(mjpeg_resolutions) / sizeof(mjpeg_resolutions[0]); i++) {
        if (mjpeg_resolutions[i].width == width && mjpeg_resolutions[i].height == height) {
            return 1;  // 找到匹配的分辨率
        }
    }
    return 0;  // 未找到匹配的分辨率
}

// 检查 YUV 格式的分辨率是否有效
int yuv_resolution_supported(uint16_t width, uint16_t height) {
    for (int i = 0; i < sizeof(yuv_resolutions) / sizeof(yuv_resolutions[0]); i++) {
        if (yuv_resolutions[i].width == width && yuv_resolutions[i].height == height) {
            return 1;  // 找到匹配的分辨率
        }
    }
    return 0;  // 未找到匹配的分辨率
}

