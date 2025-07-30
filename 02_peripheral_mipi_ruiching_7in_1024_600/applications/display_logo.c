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
#include <rtt_logo_rgb888_1024x600.h>

static void display_logo(int argc, char *argv[])
{
    rt_uint32_t bright = 200;
    struct rt_device_graphic_info info;
    struct rt_device *device = rt_device_find("lcd");

    if (!device)
    {
        rt_kprintf("Can't find device display_rgb\n");
        return;
    }

    rt_device_init(device);
    rt_device_open(device, RT_DEVICE_FLAG_RDWR);

    rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);

    rt_kprintf("Graphic Device Information:\n");
    rt_kprintf("pixel_format: %d\n", info.pixel_format);
    rt_kprintf("bits_per_pixel: %u\n", info.bits_per_pixel);
    rt_kprintf("pitch: %d bytes\n", info.pitch);
    rt_kprintf("size: %dx%d\n", info.width, info.height);
    rt_kprintf("framebuffer: %p\n", info.framebuffer);
    rt_kprintf("smem_len: %lu\n", (rt_uint32_t)info.smem_len);

    rt_device_control(device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &bright);

    rt_memcpy(info.framebuffer, img_data, info.smem_len);
    rt_device_control(device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
}
MSH_CMD_EXPORT(display_logo, display_logo);
