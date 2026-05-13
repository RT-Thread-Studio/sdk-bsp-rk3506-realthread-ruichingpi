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

struct raw_frame g_raw_frame[2] = {0};
static volatile int g_is_writing = 0;
volatile int g_fill_idx = 0;   /* 当前采集写入哪个buffer */
volatile int g_save_idx = -1;  /* 哪个buffer等待保存，-1表示没有 */
char doc_path[50];

int raw_frame_init(struct raw_frame *frame, uint32_t size)
{
    if (frame == RT_NULL)
        return -1;

    if (frame->buf != RT_NULL)
    {
        if (frame->buf_size >= size)
        {
            rt_kprintf("reuse raw frame buffer: buf=%p size=%u\n",frame->buf, frame->buf_size);
            frame->frame_len = 0;
            frame->frame_size = 0;
            frame->ready = 0;
            return 0;
        }

        rt_free(frame->buf);
        frame->buf = RT_NULL;
    }

    frame->buf = rt_malloc(size);
    if (frame->buf == RT_NULL)
    {
        rt_kprintf("malloc raw frame buffer failed, size=%u\n", size);
        frame->buf_size = 0;
        frame->frame_len = 0;
        frame->frame_size = 0;
        frame->ready = 0;
        return -1;
    }

    frame->buf_size = size;
    frame->frame_len = 0;
    frame->frame_size = 0;
    frame->ready = 0;
    rt_memset(frame->buf, 0, size);

    rt_kprintf("malloc raw frame buffer success: %u bytes, buf=%p\n",
               size, frame->buf);
    return 0;
}


void raw_frame_deinit(struct raw_frame *frame)
{
    if (frame == RT_NULL)
        return;

    if (frame->buf)
    {
        rt_free(frame->buf);
        frame->buf = RT_NULL;
    }

    frame->buf_size = 0;
    frame->frame_len = 0;
    frame->frame_size = 0;
    frame->ready = 0;
}

int raw_double_buf_init(uint32_t size)
{
    if (raw_frame_init(&g_raw_frame[0], size) < 0)
        return -1;

    if (raw_frame_init(&g_raw_frame[1], size) < 0)
    {
        raw_frame_deinit(&g_raw_frame[0]);
        return -1;
    }

    g_fill_idx = 0;
    g_save_idx = -1;
    return 0;
}

void raw_double_buf_deinit(void)
{
    raw_frame_deinit(&g_raw_frame[0]);
    raw_frame_deinit(&g_raw_frame[1]);
    g_fill_idx = 0;
    g_save_idx = -1;
}

int save_one_frame(struct raw_frame *frame, uint8_t type)
{
    int fd;
    char filename[160];

    if (frame == RT_NULL || frame->buf == RT_NULL)
        return -1;
    if (type == 1)
        rt_snprintf(filename, sizeof(filename), "%s/output.jpg", doc_path);
    else
        rt_snprintf(filename, sizeof(filename), "%s/output.yuv", doc_path);

    while (g_is_writing)
    {
        rt_thread_mdelay(1);
    }
    g_is_writing = 1;
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        rt_kprintf("[file_save] open %s failed\n", filename);
        g_is_writing = 0;
        return -1;
    }

    write(fd, frame->buf, frame->frame_size);
    close(fd);

    rt_kprintf("[file_save] saved to %s, len=%u\n", filename, frame->frame_size);

    g_is_writing = 0;
    return 0;
}


