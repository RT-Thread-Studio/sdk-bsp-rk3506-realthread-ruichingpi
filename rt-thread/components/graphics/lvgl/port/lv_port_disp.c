/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-17     Wayne        The first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <lvgl.h>

#define DBG_TAG         "lvgl.disp"
#define DBG_LEVEL       DBG_LOG
#include <rtdbg.h>

static void lvgl_flush_cb(lv_display_t *display, const lv_area_t *area,
    uint8_t *px_map)
{
    rt_uint8_t *framebuffer;
    struct rt_device_graphic_info info;
    struct rt_device *device;

    device = lv_display_get_driver_data(display);
    if (!device)
    {
        return;
    }

    framebuffer = lv_display_get_user_data(display);
    if (!framebuffer)
    {
        return;
    }

    rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);

    rt_memcpy(info.framebuffer, framebuffer,
        info.width * info.height * sizeof(lv_color_t));

    rt_device_control(device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

    lv_display_flush_ready(display);
}

void lv_port_disp_init(void)
{
    struct rt_device *device = RT_NULL;
    lv_display_t *display = RT_NULL;
    rt_uint8_t *framebuffer = RT_NULL;
    struct rt_device_graphic_info info;
    rt_uint32_t bright = 100;

    device = rt_device_find("lcd");
    if (device == RT_NULL)
    {
        LOG_E("Can't find device lcd.");
        goto __fail;
    }

    rt_device_init(device);
    rt_device_open(device, RT_DEVICE_FLAG_RDWR);

    rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);
    rt_device_control(device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &bright);

    framebuffer = rt_malloc(info.width * info.height * sizeof(lv_color_t));
    if (!framebuffer)
    {
        LOG_E("malloc fail");
        goto __fail;
    }

    display = lv_display_create(info.width, info.height);
    if (display == RT_NULL)
    {
        LOG_E("lvgl display create failed.");
        goto __fail;
    }

    lv_display_set_driver_data(display, device);
    lv_display_set_user_data(display, framebuffer);

    lv_display_set_buffers(display, framebuffer, NULL,
        info.width * info.height * sizeof(lv_color_t),
        LV_DISPLAY_RENDER_MODE_FULL);

    lv_display_set_flush_cb(display, lvgl_flush_cb);

    return;

__fail:
    if (device)
    {
        rt_device_close(device);
    }

    if (framebuffer)
    {
        rt_free(framebuffer);
    }

    if (display)
    {
        lv_display_delete(display);
    }
}
