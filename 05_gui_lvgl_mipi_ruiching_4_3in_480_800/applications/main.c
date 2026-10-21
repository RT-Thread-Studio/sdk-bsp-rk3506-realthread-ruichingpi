/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "display_subsystem.h"

#define LCD_DEVICE_NAME "lcd"
#define LCD_BACKLIGHT_MAX_BRIGHTNESS 200

extern int lvgl_thread_init(void);

int main(void)
{
    rt_uint32_t brightness = LCD_BACKLIGHT_MAX_BRIGHTNESS;
    struct rt_device *device;

    rt_kprintf("Hello, RT-Thread app\n");

    if (mipi_display_start() != RT_EOK)
    {
        rt_kprintf("mipi display start failed\n");
    }

    lvgl_thread_init();
    rt_thread_mdelay(1000);

    device = rt_device_find(LCD_DEVICE_NAME);

    if (device == RT_NULL)
    {
        rt_kprintf("LCD device not found\n");
        return -RT_ENOENT;
    }

    if (rt_device_control(device, RTGRAPHIC_CTRL_SET_BRIGHTNESS,
                          &brightness) != RT_EOK)
    {
        rt_kprintf("Failed to set LCD backlight\n");
        return -RT_ERROR;
    }

    return 0;
}
