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
#include <bmp2bgr888.h>
#include <rc_logo_4_3in_bmp.h>

int main(void)
{
    struct rt_device_graphic_info info;
    rt_uint32_t bright = 200;
    struct rt_device *device;
    const rt_uint8_t *bmp_data;
    rt_uint8_t *img_data;
    rt_err_t ret;

    rt_kprintf("Hello, RT-Thread app\n");
    rt_kprintf("[APP] Tutorial: BMP to MIPI display workflow.\n");

    rt_kprintf("Step 1 - Prepare BMP data. ok!\n");
    bmp_data = logo_bmp_480_800_data;

    rt_kprintf("Step 2 - BMP to BGR888 Conversion. ");
    img_data = rt_malloc(480 * 800 * 3);
    if (!img_data)
    {
        rt_kprintf("Malloc img data failed.\n");
        return (-RT_ENOMEM);
    }

    ret = bmp_extract_bgr888(bmp_data, img_data);
    if (ret)
    {
        rt_kprintf("Bmp to BRG888 failed.\n");
        return ret;
    }
    rt_kprintf("ok!\n");

    rt_kprintf("Step 3 - Init LCD. ");
    device = rt_device_find("lcd");
    if (!device)
    {
        rt_kprintf("Can't find device lcd\n");
        return (-RT_ENOENT);
    }
    rt_kprintf("ok!\n");

    rt_device_init(device);
    rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);

    rt_kprintf("    Graphic Device Information:\n");
    rt_kprintf("    pixel_format: %d\n", info.pixel_format);
    rt_kprintf("    bits_per_pixel: %u\n", info.bits_per_pixel);
    rt_kprintf("    pitch: %d bytes\n", info.pitch);
    rt_kprintf("    size: %dx%d\n", info.width, info.height);
    rt_kprintf("    framebuffer: %p\n", info.framebuffer);
    rt_kprintf("    smem_len: %lu\n", (rt_uint32_t)info.smem_len);

    rt_kprintf("Step 4 - Set backlight open. ok!\n");
    rt_device_control(device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &bright);

    rt_kprintf("Step 5 - Frame update. ok!\n");
    rt_memcpy(info.framebuffer, img_data, info.smem_len);
    rt_device_control(device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

    rt_free(img_data);

    return 0;
}
