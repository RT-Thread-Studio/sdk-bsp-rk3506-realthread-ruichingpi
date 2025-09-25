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

#define JD_WORKSPACE_SIZE 8192
rt_uint8_t tjpgd_work[JD_WORKSPACE_SIZE];

#define MJPEG_SRC_WIDTH   640
#define MJPEG_SRC_HEIGHT  480

unsigned char *g_img_buf = NULL;
int g_img_size = 0;

uint8_t *dec_rgb_buf = NULL;
size_t dec_rgb_buf_size = 0;
uint8_t *scale_buf = NULL;
size_t scale_buf_size = 0;

size_t input_func(JDEC *jdec, uint8_t *buff, size_t ndata)
{
    struct jpeg_input_ctx *ctx = (struct jpeg_input_ctx *) jdec->device;
    size_t remain = ctx->jpeg_size - ctx->offset;
    if (ndata > remain)
        ndata = (size_t) remain;
    if (buff && ndata)
        memcpy(buff, ctx->jpeg_data + ctx->offset, ndata);
    ctx->offset += ndata;
    return ndata;
}

int output_func(JDEC *jdec, void *bitmap, JRECT *rect)
{
    int bw = rect->right - rect->left + 1;
    int bh = rect->bottom - rect->top + 1;
    uint8_t *src = (uint8_t *) bitmap;
    int src_w = MJPEG_SRC_WIDTH;

    if (!dec_rgb_buf)
        return 0;

    for (int y = 0; y < bh; y++)
    {
        int dst_y = rect->top + y;
        uint8_t *dst = dec_rgb_buf + (dst_y * src_w + rect->left) * 3;
        memcpy(dst, src + y * bw * 3, bw * 3);
    }
    return 1;
}

void rgb_nearest_scale(const uint8_t *src_rgb, int src_w, int src_h, uint8_t *dst_rgb, int dst_w, int dst_h)
{
    for (int dy = 0; dy < dst_h; dy++)
    {
        int src_y = (dy * src_h) / dst_h;
        const uint8_t *src_row = src_rgb + (src_y * src_w) * 3;
        uint8_t *dst_row = dst_rgb + (dy * dst_w) * 3;
        for (int dx = 0; dx < dst_w; dx++)
        {
            int src_x = (dx * src_w) / dst_w;
            const uint8_t *p = src_row + src_x * 3;
            uint8_t *q = dst_row + dx * 3;
            q[0] = p[0];
            q[1] = p[1];
            q[2] = p[2];
        }
    }
}

int load_image(const char *path, int max_size)
{
    int fd, ret;
    int total = 0;

    if (g_img_buf)
    {
        rt_free(g_img_buf);
        g_img_buf = NULL;
    }
    g_img_buf = (unsigned char*) rt_malloc(max_size);
    g_img_size = 0;

    if (!g_img_buf)
        return -1;

    fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0)
        return -1;

    while (total < max_size)
    {
        ret = read(fd, g_img_buf + total, max_size - total);
        if (ret <= 0)
            break;
        total += ret;
    }

    close(fd);
    g_img_size = (ret < 0) ? 0 : total;
    return g_img_size;
}

unsigned char *get_image_data(void)
{
    return g_img_buf;
}

int get_image_size(void)
{
    return g_img_size;
}

