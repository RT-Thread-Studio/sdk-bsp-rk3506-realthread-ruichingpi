/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "jpeg.h"
#include "yolo.hpp"
#include <rtthread.h>
#include <rtdevice.h>
#include <unistd.h>
#include <fcntl.h>

#define RT_UVC_CTRL_SET_CALLBACK    0x0
#define RT_UVC_CTRL_START_STREAM    0x1
#define RT_UVC_CTRL_STOP_STREAM     0x2
#define RT_UVC_CTRL_RELEASE_FRAME   0x3

static struct rt_device *uvc_device = NULL;
static struct rt_device *lcd_device = NULL;
static struct rt_device_graphic_info lcd_info;
static volatile rt_bool_t is_streaming = RT_FALSE;
static rt_uint8_t *lcd_fb = RT_NULL;

struct usbh_videoframe
{
    uint8_t *frame_buf;
    uint32_t frame_bufsize;
    uint32_t frame_format;
    uint32_t frame_size;
};

typedef void (*frame_callback_t)(struct usbh_videoframe *frame);
struct usbh_videoframe *g_global_frame = NULL;
char file_path[50];
static struct rt_semaphore sem_lock;
bool flag = false;
static YOLONCNN* global_yolo = nullptr;  // 全局YOLO实例
int frame_i = 1;

static rt_err_t init_lcd_device(void)
{
    rt_uint32_t bright = 200;

    lcd_device = rt_device_find("lcd");
    if (!lcd_device)
        return -RT_ENOENT;
    if (rt_device_init(lcd_device) != RT_EOK)
        return -RT_ERROR;
    if (rt_device_open(lcd_device, RT_DEVICE_FLAG_RDWR) != RT_EOK)
        return -RT_ERROR;
    if (rt_device_control(lcd_device, RTGRAPHIC_CTRL_GET_INFO, &lcd_info) != RT_EOK)
        return -RT_ERROR;
    rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &bright);
    lcd_fb = (rt_uint8_t *) lcd_info.framebuffer;
    rt_kprintf("[LCD] Initialized: %dx%d, pitch=%d\n", lcd_info.width, lcd_info.height, lcd_info.pitch);

    return RT_EOK;
}

void my_function(struct usbh_videoframe *frame)
{
    frame_i++;
    if (frame_i == 10)
    {
        g_global_frame = frame;
        rt_sem_release(&sem_lock);
        frame_i = 0;
    }
}

static int uvc_ai_start(void)
{
    uint8_t type = 1;

    if (init_lcd_device() != RT_EOK)
        return -RT_ERROR;

    uvc_device = rt_device_find("uvc");

    if (!uvc_device)
    {
        rt_kprintf("Can't find device uvc\n");
        return (-RT_ERROR);
    }

    rt_kprintf("uvc capture mjpeg type picture\r\n");

    frame_callback_t my_callback = my_function;
    rt_sem_init(&sem_lock, "lock", 0, RT_IPC_FLAG_PRIO);

    rt_device_init(uvc_device);
    rt_device_open(uvc_device, RT_DEVICE_FLAG_RDWR);

    rt_device_control(uvc_device, RT_UVC_CTRL_SET_CALLBACK, (void *) my_callback);
    rt_device_control(uvc_device, RT_UVC_CTRL_START_STREAM, &type);

    rt_kprintf("Initializing YOLO model...\n");
    global_yolo = new YOLONCNN("/tmp/model2.param", "/tmp/model2.bin");
    if (!global_yolo)
    {
        rt_kprintf("Failed to initialize YOLO model\n");
        return (-RT_ERROR);
    }

    is_streaming = RT_TRUE;

    while (is_streaming)
    {

        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        uint time_start = rt_tick_get();

        std::vector<uchar> img_vec;
        if (g_global_frame != nullptr && g_global_frame->frame_buf != nullptr && g_global_frame->frame_size > 0)
        {
            img_vec.reserve(g_global_frame->frame_size);
            img_vec.assign(g_global_frame->frame_buf, g_global_frame->frame_buf + g_global_frame->frame_size);
        }
        else
        {
            rt_kprintf("The frame data is invalid and cannot be stored in vector\n");
        }
        cv::Mat img = cv::imdecode(img_vec, cv::IMREAD_COLOR);

        if (img.empty())
        {
            rt_kprintf("Failed to read captured image: %s\n", file_path);
        }
        else
        {
            auto boxes = global_yolo->detect(img);

            bool encode_success = global_yolo->visualize_to_buf(img, boxes, ".jpg", 90);
            if (!encode_success)
            {
                rt_kprintf("The image encoding to the buffer failed！\n");
                return -1;
            }
        }

        struct jpeg_input_ctx jpeg_ctx = { .jpeg_data = g_img_buf, .jpeg_size = g_img_size, .offset = 0, };

        JDEC jdec;
        JRESULT res = jd_prepare(&jdec, input_func, tjpgd_work, JD_WORKSPACE_SIZE, &jpeg_ctx);
        if (res != JDR_OK)
        {
            rt_kprintf("[UVC] jd_prepare error: %d\n", res);
            return -RT_ERROR;
        }

        int src_w = MJPEG_SRC_WIDTH;
        int src_h = MJPEG_SRC_HEIGHT;
        size_t needed = (size_t) src_w * src_h * 3;

        if (needed > dec_rgb_buf_size)
        {
            if (dec_rgb_buf)
                rt_free(dec_rgb_buf);
            dec_rgb_buf = (uint8_t *) rt_malloc(needed);
            if (!dec_rgb_buf)
            {
                rt_kprintf("[UVC] alloc dec_rgb_buf failed\n");
                return -RT_ERROR;
            }
            dec_rgb_buf_size = needed;
        }

        res = jd_decomp(&jdec, output_func, 0);
        if (res != JDR_OK)
        {
            rt_kprintf("[UVC] jd_decomp error: %d\n", res);

            return -RT_ERROR;
        }

        int dst_w = lcd_info.width;
        int dst_h = lcd_info.height;
        size_t need_scale_size = (size_t) dst_w * dst_h * 3;

        if (need_scale_size > scale_buf_size)
        {
            if (scale_buf)
                rt_free(scale_buf);
            scale_buf = (uint8_t *) rt_malloc(need_scale_size);
            if (!scale_buf)
            {
                rt_kprintf("[UVC] alloc scale_buf failed\n");
                return -RT_ERROR;
            }
            scale_buf_size = need_scale_size;
        }

        rgb_nearest_scale(dec_rgb_buf, src_w, src_h, scale_buf, dst_w, dst_h);

        for (int y = 0; y < dst_h; y++)
        {
            uint8_t *dst_row = lcd_fb + ((dst_h - 1 - y) * lcd_info.pitch);
            uint8_t *src_row = scale_buf + y * dst_w * 3;
            memcpy(dst_row, src_row, dst_w * 3);
        }

        rt_device_control(lcd_device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);

        uint time_end = rt_tick_get();
        float fps = (1000.0f / (time_end - time_start));
        rt_kprintf("fps: %.2f\r\n", fps);

        if (g_img_buf)
        {
            rt_free(g_img_buf);
            g_img_buf = NULL;
            g_img_size = 0;
        }
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(uvc_ai_start, uvc_ai_start);

