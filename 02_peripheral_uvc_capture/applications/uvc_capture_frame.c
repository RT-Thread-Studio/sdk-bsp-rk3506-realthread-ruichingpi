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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <uvc_buffer.h>
#include <resolutions_check.h>

#define RT_UVC_CTRL_SET_CALLBACK          0x0             /**< set callback control command */
#define RT_UVC_CTRL_START_STREAM          0x1             /**< set start stream control command */
#define RT_UVC_CTRL_STOP_STREAM           0x2             /**< set stop stream control command */
#define RT_UVC_CTRL_SET_RESOLUTION        0x3             /**< set resolution control command */
#define RT_UVC_CTRL_SET_RAW_CALLBACK      0x4            /**< set raw_callback control command */


typedef void (*raw_frame_callback_t)(struct uvc_raw_packet *);

static volatile int g_stream_on = 0;
static volatile int g_drop_until_eof = 0;
static volatile uint8_t g_raw_type = 0;
static struct rt_semaphore g_raw_sem;
static rt_bool_t g_raw_sem_inited = RT_FALSE;

/* raw packet 回调 */
void raw_my_function(struct uvc_raw_packet *pkt)
{
    struct raw_frame *frame;
    int next_idx;

    if (pkt == RT_NULL)
        return;

    if (!g_stream_on)
        return;

    /* 当前这一整帧已经判坏：一直丢到 EOF */
    if (g_drop_until_eof)
    {
        if (pkt->eof)
        {
            g_drop_until_eof = 0;
        }
        return;
    }

    frame = &g_raw_frame[g_fill_idx];

    if (frame->buf == RT_NULL)
        return;

    /* 当前buffer还没腾出来，这一整帧接不住了：整帧丢弃到 EOF */
    if (frame->ready)
    {
        g_drop_until_eof = 1;
        return;
    }

    if (pkt->error)
    {
        frame->frame_len = 0;
        g_drop_until_eof = 1;
        return;
    }

    if ((pkt->payload == RT_NULL) || (pkt->payload_len == 0))
        return;

    /* 一帧的第一个有效packet必须从 offset 0 开始，否则说明前面已经丢了 */
    if ((frame->frame_len == 0) && (pkt->frame_offset != 0))
    {
        g_drop_until_eof = 1;
        return;
    }

    if ((pkt->frame_offset + pkt->payload_len) > frame->buf_size)
    {
        rt_kprintf("frame buffer overflow: idx=%d offset=%u len=%u\n",
                   g_fill_idx, pkt->frame_offset, pkt->payload_len);
        frame->frame_len = 0;
        g_drop_until_eof = 1;
        return;
    }

    rt_memcpy(frame->buf + pkt->frame_offset,
              pkt->payload,
              pkt->payload_len);

    frame->frame_len = pkt->frame_offset + pkt->payload_len;
    if (pkt->eof)
    {
        /* 只有 MJPEG 才检查 JPEG 头尾 */
        if (g_raw_type == 1)
        {
            if (frame->frame_len < 4 ||
                frame->buf[0] != 0xFF || frame->buf[1] != 0xD8 ||
                frame->buf[frame->frame_len - 2] != 0xFF ||
                frame->buf[frame->frame_len - 1] != 0xD9)
            {
                frame->frame_len = 0;
                return;
            }
        }

        frame->frame_size = frame->frame_len;
        frame->frame_len = 0;
        frame->ready = 1;

        g_save_idx = g_fill_idx;

        next_idx = (g_fill_idx + 1) % 2;
        if (g_raw_frame[next_idx].ready == 0)
        {
            g_fill_idx = next_idx;
        }
        else
        {
            g_drop_until_eof = 1;
        }

        if (g_raw_sem_inited)
        {
            rt_sem_release(&g_raw_sem);
        }
    }
}

/* raw 帧链路：uvc_capture_frame <type> <w> <h> <path> */
static int uvc_capture_frame(int argc, char *argv[])
{
    uint8_t type;
    rt_uint16_t width, height;
    struct rt_uvc_resolution res;
    struct rt_device *device = rt_device_find("uvc");

    if (!device)
    {
        rt_kprintf("Can't find device uvc\n");
        return -1;
    }

    if (argc != 5)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("  uvc_capture_frame <type> <w> <h> <path>\n");
        rt_kprintf("  type: 0=YUYV, 1=MJPEG\n");
        return -1;
    }

    type = (uint8_t)atoi(argv[1]);
    width = (rt_uint16_t)atoi(argv[2]);
    height = (rt_uint16_t)atoi(argv[3]);

    if (type != 0 && type != 1)
    {
        rt_kprintf("type must be 0 (YUYV) or 1 (MJPEG)\n");
        return -1;
    }

    if ((width == 0) || (height == 0))
    {
        rt_kprintf("invalid resolution: %d x %d\r\n", width, height);
        return -1;
    }
    if(type == 1){
        if (!mjpeg_resolution_supported(width, height)) {
            rt_kprintf("invalid resolution\r\n");
            return -1;}
    }
    if(type == 0){
        if (!yuv_resolution_supported(width, height)) {
            rt_kprintf("invalid resolution\r\n");
            return -1;}
    }
    if (access(argv[4], F_OK) != 0)
    {
        rt_kprintf("file path does not exist\r\n");
        return -1;
    }

    rt_snprintf(doc_path, sizeof(doc_path), "%s", argv[4]);

    res.width = width;
    res.height = height;

    if (raw_double_buf_init(width * height * 2) < 0)
        return -1;

    if (!g_raw_sem_inited)
    {
        if (rt_sem_init(&g_raw_sem, "rawsem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)
        {
            rt_kprintf("init raw sem failed\n");
            raw_double_buf_deinit();
            return -1;
        }
        g_raw_sem_inited = RT_TRUE;
    }

    g_fill_idx = 0;
    g_save_idx = -1;
    g_stream_on = 1;
    g_drop_until_eof = 0;
    g_raw_type = type;

    raw_frame_callback_t raw_callback = raw_my_function;

    rt_device_init(device);
    rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    rt_device_control(device, RT_UVC_CTRL_SET_RESOLUTION, &res);

    rt_device_control(device, RT_UVC_CTRL_SET_RAW_CALLBACK, (void *)raw_my_function);
    while (rt_sem_take(&g_raw_sem, 0) == RT_EOK)
    {
        /* drain stale semaphore count */
    }
    rt_device_control(device, RT_UVC_CTRL_START_STREAM, &type);

    rt_kprintf("uvc raw stream start: %d x %d, continuous save mode\r\n",
               width, height);
    if (rt_sem_take(&g_raw_sem, RT_WAITING_FOREVER) == RT_EOK)
    {
        for (int i = 0; i < 2; i++)
        {
            if (g_raw_frame[i].ready)
            {
                save_one_frame(&g_raw_frame[i], type);
                g_raw_frame[i].ready = 0;
                break;
            }
        }
    }

    g_stream_on = 0;
    rt_device_control(device, RT_UVC_CTRL_STOP_STREAM, NULL);

    g_drop_until_eof = 0;
    g_fill_idx = 0;
    g_save_idx = -1;

    g_raw_frame[0].ready = 0;
    g_raw_frame[0].frame_len = 0;
    g_raw_frame[0].frame_size = 0;

    g_raw_frame[1].ready = 0;
    g_raw_frame[1].frame_len = 0;
    g_raw_frame[1].frame_size = 0;

    return 0;
}

MSH_CMD_EXPORT(uvc_capture_frame, uvc raw frame capture yuv/mjpeg);


