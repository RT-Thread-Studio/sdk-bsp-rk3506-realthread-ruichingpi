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
#include <rtconfig.h>
#include <rtdevice.h>
#include <lvgl.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef RT_USING_RGA
#include <rga.h>
#include <im2d.h>
#endif

#define DBG_TAG         "lvgl.disp"
#define DBG_LEVEL       DBG_LOG
#include <rtdbg.h>
#include "display_subsystem.h"

#ifndef LVGL_DISP_PARTIAL_LINES
#define LVGL_DISP_PARTIAL_LINES 160
#endif

/* RGB666* (dsi,format=1/2): 18-bit pixels in a 24-bit framebuffer. */
static rt_bool_t g_pack666;

static void pack666_px(rt_uint8_t *p, rt_uint8_t b, rt_uint8_t g, rt_uint8_t r)
{
    rt_uint32_t w = ((rt_uint32_t)(b >> 2) << 12) |
                    ((rt_uint32_t)(g >> 2) << 6) |
                    (rt_uint32_t)(r >> 2);

    p[0] = (rt_uint8_t)w;
    p[1] = (rt_uint8_t)(w >> 8);
    p[2] = (rt_uint8_t)(w >> 16);
}

/* Internal pixel format ids used by software blit */
enum lvgl_px_fmt
{
    LVGL_PX_RGB565 = 0,
    LVGL_PX_BGR565,
    LVGL_PX_RGB888,
    LVGL_PX_BGR888,
    LVGL_PX_RGB666,
    LVGL_PX_BGR666,
    LVGL_PX_XRGB8888,
    LVGL_PX_ARGB8888,
    LVGL_PX_ABGR8888,
    LVGL_PX_UNKNOWN
};

struct lvgl_disp_ctx
{
    struct rt_device *device;
    struct rt_device_graphic_info info;
    rt_uint8_t *draw_buf;
    rt_uint32_t draw_buf_size;
    enum lvgl_px_fmt lv_fmt;
    enum lvgl_px_fmt fb_fmt;
    rt_bool_t same_fmt;
#ifdef RT_USING_RGA
    rt_bool_t rga_ok;
    rt_bool_t rga_warned;
#endif
};

static struct lvgl_disp_ctx g_disp_ctx;

static int px_fmt_bpp(enum lvgl_px_fmt fmt);
static const char *px_fmt_name(enum lvgl_px_fmt fmt);

static int px_fmt_bpp(enum lvgl_px_fmt fmt)
{
    switch (fmt)
    {
    case LVGL_PX_RGB565:
    case LVGL_PX_BGR565:
        return 16;
    case LVGL_PX_RGB888:
    case LVGL_PX_BGR888:
    case LVGL_PX_RGB666:
    case LVGL_PX_BGR666:
        return 24;
    case LVGL_PX_XRGB8888:
    case LVGL_PX_ARGB8888:
    case LVGL_PX_ABGR8888:
    default:
        return 32;
    }
}

static const char *px_fmt_name(enum lvgl_px_fmt fmt)
{
    switch (fmt)
    {
    case LVGL_PX_RGB565:   return "RGB565";
    case LVGL_PX_BGR565:   return "BGR565";
    case LVGL_PX_RGB888:   return "RGB888";
    case LVGL_PX_BGR888:   return "BGR888";
    case LVGL_PX_RGB666:   return "RGB666";
    case LVGL_PX_BGR666:   return "BGR666";
    case LVGL_PX_XRGB8888: return "XRGB8888";
    case LVGL_PX_ARGB8888: return "ARGB8888";
    case LVGL_PX_ABGR8888: return "ABGR8888";
    default:               return "UNKNOWN";
    }
}

static enum lvgl_px_fmt rtgraphic_to_px(rt_uint8_t pixel_format)
{
    switch (pixel_format)
    {
    case RTGRAPHIC_PIXEL_FORMAT_RGB565:
        return LVGL_PX_RGB565;
    /* BGR565 aliases RGB565P in graphic.h — same enum value */
    case RTGRAPHIC_PIXEL_FORMAT_BGR565:
        return LVGL_PX_BGR565;
    case RTGRAPHIC_PIXEL_FORMAT_RGB666:
        return LVGL_PX_RGB666;
    case RTGRAPHIC_PIXEL_FORMAT_BGR666:
        return LVGL_PX_BGR666;
    case RTGRAPHIC_PIXEL_FORMAT_RGB888:
        return LVGL_PX_RGB888;
    case RTGRAPHIC_PIXEL_FORMAT_BGR888:
        return LVGL_PX_BGR888;
    case RTGRAPHIC_PIXEL_FORMAT_ARGB888:
        return LVGL_PX_ARGB8888;
    case RTGRAPHIC_PIXEL_FORMAT_ABGR888:
        return LVGL_PX_ABGR8888;
    default:
        return LVGL_PX_UNKNOWN;
    }
}

static lv_color_format_t px_to_lv_cf(enum lvgl_px_fmt fmt)
{
    switch (fmt)
    {
    case LVGL_PX_RGB565:   return LV_COLOR_FORMAT_RGB565;
    case LVGL_PX_RGB888:   return LV_COLOR_FORMAT_RGB888;
    case LVGL_PX_BGR888:   return LV_COLOR_FORMAT_RGB888;
    case LVGL_PX_RGB666:   return LV_COLOR_FORMAT_RGB888;
    case LVGL_PX_BGR666:   return LV_COLOR_FORMAT_RGB888;
    case LVGL_PX_ARGB8888: return LV_COLOR_FORMAT_ARGB8888;
    case LVGL_PX_XRGB8888:
    default:               return LV_COLOR_FORMAT_XRGB8888;
    }
}

#ifdef RT_USING_RGA
static int px_to_rga_fmt(enum lvgl_px_fmt fmt)
{
    switch (fmt)
    {
    case LVGL_PX_RGB565:   return RK_FORMAT_RGB_565;
    case LVGL_PX_BGR565:   return RK_FORMAT_BGR_565;
    case LVGL_PX_RGB888:   return RK_FORMAT_RGB_888;
    case LVGL_PX_BGR888:   return RK_FORMAT_BGR_888;
    case LVGL_PX_RGB666:   return RK_FORMAT_RGB_888;
    case LVGL_PX_BGR666:   return RK_FORMAT_BGR_888;
    case LVGL_PX_ARGB8888: return RK_FORMAT_RGBA_8888;
    case LVGL_PX_ABGR8888: return RK_FORMAT_BGRA_8888;
    case LVGL_PX_XRGB8888:
    default:               return RK_FORMAT_RGBX_8888;
    }
}
#endif /* RT_USING_RGA */

static void px_unpack(enum lvgl_px_fmt fmt, const rt_uint8_t *p,
                      rt_uint8_t *r, rt_uint8_t *g, rt_uint8_t *b)
{
    switch (fmt)
    {
    case LVGL_PX_RGB565:
    {
        rt_uint16_t c = (rt_uint16_t)p[0] | ((rt_uint16_t)p[1] << 8);
        *r = (rt_uint8_t)(((c >> 11) & 0x1f) << 3);
        *g = (rt_uint8_t)(((c >> 5) & 0x3f) << 2);
        *b = (rt_uint8_t)((c & 0x1f) << 3);
        break;
    }
    case LVGL_PX_BGR565:
    {
        /* high bits = blue */
        rt_uint16_t c = (rt_uint16_t)p[0] | ((rt_uint16_t)p[1] << 8);
        *b = (rt_uint8_t)(((c >> 11) & 0x1f) << 3);
        *g = (rt_uint8_t)(((c >> 5) & 0x3f) << 2);
        *r = (rt_uint8_t)((c & 0x1f) << 3);
        break;
    }
    case LVGL_PX_RGB888:
        *r = p[0]; *g = p[1]; *b = p[2];
        break;
    case LVGL_PX_BGR888:
        *b = p[0]; *g = p[1]; *r = p[2];
        break;
    case LVGL_PX_RGB666:
    case LVGL_PX_BGR666:
    {
        rt_uint32_t w = (rt_uint32_t)p[0] |
                        ((rt_uint32_t)p[1] << 8) |
                        ((rt_uint32_t)p[2] << 16);
        rt_uint8_t c0 = (rt_uint8_t)(((w >> 12) & 0x3f) << 2);
        rt_uint8_t c1 = (rt_uint8_t)(((w >> 6) & 0x3f) << 2);
        rt_uint8_t c2 = (rt_uint8_t)((w & 0x3f) << 2);

        if (fmt == LVGL_PX_RGB666)
        {
            *r = c0;
            *g = c1;
            *b = c2;
        }
        else
        {
            *b = c0;
            *g = c1;
            *r = c2;
        }
        break;
    }
    case LVGL_PX_XRGB8888:
    case LVGL_PX_ARGB8888:
        /* little-endian 0xAARRGGBB → bytes B,G,R,A */
        *b = p[0]; *g = p[1]; *r = p[2];
        break;
    case LVGL_PX_ABGR8888:
        /* bytes R,G,B,A */
        *r = p[0]; *g = p[1]; *b = p[2];
        break;
    default:
        *r = *g = *b = 0;
        break;
    }
}

static void px_pack(enum lvgl_px_fmt fmt, rt_uint8_t *p,
                    rt_uint8_t r, rt_uint8_t g, rt_uint8_t b)
{
    rt_uint16_t c;

    switch (fmt)
    {
    case LVGL_PX_RGB565:
        c = (rt_uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        p[0] = (rt_uint8_t)c;
        p[1] = (rt_uint8_t)(c >> 8);
        break;
    case LVGL_PX_BGR565:
        c = (rt_uint16_t)(((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3));
        p[0] = (rt_uint8_t)c;
        p[1] = (rt_uint8_t)(c >> 8);
        break;
    case LVGL_PX_RGB888:
    case LVGL_PX_BGR888:
    case LVGL_PX_XRGB8888:
    case LVGL_PX_ARGB8888:
        if (fmt == LVGL_PX_RGB888)
        {
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
        else
        {
            p[0] = b;
            p[1] = g;
            p[2] = r;
        }
        if (fmt == LVGL_PX_XRGB8888 || fmt == LVGL_PX_ARGB8888)
            p[3] = 0xff;
        break;
    case LVGL_PX_RGB666:
        /* R6G6B6 in bits [17:0]. */
        pack666_px(p, r, g, b);
        break;
    case LVGL_PX_BGR666:
        /* B6G6R6 in bits [17:0]. */
        pack666_px(p, b, g, r);
        break;
    case LVGL_PX_ABGR8888:
        p[0] = r;
        p[1] = g;
        p[2] = b;
        p[3] = 0xff;
        break;
    default:
        break;
    }
}

/* Software blit of an area; src is tightly packed for the area, dst uses pitch */
static void lvgl_sw_blit_area(const rt_uint8_t *src, rt_uint8_t *fb,
                              rt_uint32_t pitch,
                              enum lvgl_px_fmt src_fmt, enum lvgl_px_fmt dst_fmt,
                              int x, int y, int w, int h)
{
    int src_bpp = px_fmt_bpp(src_fmt);
    int dst_bpp = px_fmt_bpp(dst_fmt);
    int row, col;

    if (w <= 0 || h <= 0 || !src || !fb)
    {
        return;
    }

    for (row = 0; row < h; row++)
    {
        const rt_uint8_t *srow = src + (rt_uint32_t)row * (rt_uint32_t)w * (src_bpp / 8);
        rt_uint8_t *drow = fb + (rt_uint32_t)(y + row) * pitch + (rt_uint32_t)x * (dst_bpp / 8);

        if (src_fmt == dst_fmt && !g_pack666)
        {
            memcpy(drow, srow, (rt_size_t)w * (dst_bpp / 8));
            continue;
        }

        for (col = 0; col < w; col++)
        {
            rt_uint8_t r, g, b;
            px_unpack(src_fmt, srow + col * (src_bpp / 8), &r, &g, &b);
            px_pack(dst_fmt, drow + col * (dst_bpp / 8), r, g, b);
        }
    }
}

#ifdef RT_USING_RGA
static rt_err_t lvgl_rga_blit_area(struct lvgl_disp_ctx *ctx,
                                   const rt_uint8_t *src,
                                   const lv_area_t *area)
{
    rga_buffer_t src_img;
    rga_buffer_t dst_img;
    im_rect src_rect;
    im_rect dst_rect;
    rt_uint8_t *dst_ptr;
    int w = area->x2 - area->x1 + 1;
    int h = area->y2 - area->y1 + 1;
    int dst_bpp = ctx->info.bits_per_pixel;
    int dst_stride_px;
    int ret;

    if (!ctx->rga_ok || w <= 0 || h <= 0)
    {
        return -RT_ERROR;
    }

    if (dst_bpp <= 0)
    {
        dst_bpp = px_fmt_bpp(ctx->fb_fmt);
    }
    if (dst_bpp < 8)
    {
        return -RT_ERROR;
    }

    /*
     * pitch is bytes/line. RGA wstride/hstride are in PIXELS.
     * wrapbuffer_physicaladdr(phy, w, h, format[, wstride, hstride])
     * — format is 4th arg, NOT wstride. Use _t API to avoid mixing them.
     * (Wrong order put RK_FORMAT_* into hstride, e.g. BGR_888=1792.)
     */
    {
        int px = dst_bpp / 8;
        rt_uint32_t pitch = ctx->info.pitch;
        rt_uint32_t min_pitch = (rt_uint32_t)ctx->info.width * (rt_uint32_t)px;

        if (pitch < min_pitch)
        {
            pitch = min_pitch;
        }

        dst_stride_px = (int)(pitch / (rt_uint32_t)px);
        if (dst_stride_px < w)
        {
            dst_stride_px = w;
        }

        dst_ptr = (rt_uint8_t *)ctx->info.framebuffer +
                  (rt_uint32_t)area->y1 * pitch +
                  (rt_uint32_t)area->x1 * (rt_uint32_t)px;

        src_img = wrapbuffer_physicaladdr_t((void *)src, w, h, w, h,
            px_to_rga_fmt(ctx->lv_fmt));
        dst_img = wrapbuffer_physicaladdr_t(dst_ptr, w, h, dst_stride_px, h,
            px_to_rga_fmt(ctx->fb_fmt));
    }

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = w;
    src_rect.height = h;
    dst_rect = src_rect;

    ret = imcheck(src_img, dst_img, src_rect, dst_rect);
    if (ret != IM_STATUS_NOERROR)
    {
        if (!ctx->rga_warned)
        {
            LOG_W("RGA imcheck failed (%s), fallback software",
                imStrError((IM_STATUS)ret));
            ctx->rga_warned = RT_TRUE;
        }
        ctx->rga_ok = RT_FALSE;
        return -RT_ERROR;
    }

    ret = imcopy(src_img, dst_img);
    if (ret != IM_STATUS_SUCCESS)
    {
        if (!ctx->rga_warned)
        {
            LOG_W("RGA imcopy failed (%s), fallback software",
                imStrError((IM_STATUS)ret));
            ctx->rga_warned = RT_TRUE;
        }
        ctx->rga_ok = RT_FALSE;
        return -RT_ERROR;
    }

    return RT_EOK;
}
#endif /* RT_USING_RGA */

static void lvgl_flush_cb(lv_display_t *display, const lv_area_t *area,
    uint8_t *px_map)
{
    struct lvgl_disp_ctx *ctx = lv_display_get_user_data(display);
    int w, h;

    if (!ctx || !area || !px_map || !ctx->info.framebuffer)
    {
        return;
    }

    w = area->x2 - area->x1 + 1;
    h = area->y2 - area->y1 + 1;
    if (w <= 0 || h <= 0)
    {
        lv_display_flush_ready(display);
        return;
    }

#ifdef RT_USING_RGA
    /* RGA cannot pack RGB666 — software blit for 24bpp dest */
    if (ctx->rga_ok && !g_pack666)
    {
        if (lvgl_rga_blit_area(ctx, px_map, area) == RT_EOK)
        {
            rt_device_control(ctx->device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
            lv_display_flush_ready(display);
            return;
        }
    }
#endif

    lvgl_sw_blit_area(px_map, ctx->info.framebuffer,
        (ctx->info.pitch >= (rt_uint32_t)ctx->info.width *
            (rt_uint32_t)(px_fmt_bpp(ctx->fb_fmt) / 8)) ?
            ctx->info.pitch :
            (rt_uint32_t)ctx->info.width *
                (rt_uint32_t)(px_fmt_bpp(ctx->fb_fmt) / 8),
        ctx->lv_fmt, ctx->fb_fmt,
        area->x1, area->y1, w, h);

    rt_device_control(ctx->device, RTGRAPHIC_CTRL_RECT_UPDATE, RT_NULL);
    lv_display_flush_ready(display);
}

rt_err_t lv_port_disp_init(void)
{
    struct lvgl_disp_ctx *ctx = &g_disp_ctx;
    struct rt_device *device = RT_NULL;
    lv_display_t *display = RT_NULL;
    struct rt_device_graphic_info *info;
    rt_uint32_t bright = 100;
    rt_uint32_t lv_cf_size;
    rt_err_t ret = RT_EOK;
    lv_color_format_t lv_cf;

    rt_memset(ctx, 0, sizeof(*ctx));
#ifdef RT_USING_RGA
    ctx->rga_ok = RT_TRUE;
#endif

    device = rt_device_find("lcd");
    if (device == RT_NULL)
    {
        LOG_E("Can't find device lcd.");
        ret = -RT_ENOENT;
        goto __fail;
    }

    ret = rt_device_init(device);
    if (ret != RT_EOK)
    {
        LOG_E("Initialize device lcd failed: %d.", ret);
        goto __fail;
    }

    ret = rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("Open device lcd failed: %d.", ret);
        goto __fail;
    }

    ret = rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &ctx->info);
    if (ret != RT_EOK)
    {
        LOG_E("Get device lcd information failed: %d.", ret);
        goto __fail;
    }
    info = &ctx->info;

    rt_device_control(device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &bright);

    {
        rt_uint32_t dsi_fmt = 0;
        rt_device_control(device, RTGRAPHIC_CTRL_GET_DSI_FMT, &dsi_fmt);
        g_pack666 = (dsi_fmt == 1 || dsi_fmt == 2) ? RT_TRUE : RT_FALSE;
    }

    ctx->fb_fmt = rtgraphic_to_px(info->pixel_format);
    if (ctx->fb_fmt == LVGL_PX_UNKNOWN)
    {
        if (info->bits_per_pixel == 16)
        {
            ctx->fb_fmt = LVGL_PX_RGB565;
        }
        else if (info->bits_per_pixel == 24)
        {
            ctx->fb_fmt = LVGL_PX_RGB888;
        }
        else
        {
            ctx->fb_fmt = LVGL_PX_XRGB8888;
        }
        LOG_W("Unknown graphic pixel_format %u, infer bpp=%u → %s",
            info->pixel_format, info->bits_per_pixel,
            px_fmt_name(ctx->fb_fmt));
    }

    /* LVGL render format follows LV_COLOR_DEPTH / Kconfig */
    if (LV_COLOR_DEPTH == 16)
    {
        ctx->lv_fmt = LVGL_PX_RGB565;
    }
    else if (LV_COLOR_DEPTH == 24)
    {
        ctx->lv_fmt = LVGL_PX_RGB888;
    }
    else
    {
        ctx->lv_fmt = LVGL_PX_XRGB8888;
    }

    ctx->same_fmt = (ctx->lv_fmt == ctx->fb_fmt) ? RT_TRUE : RT_FALSE;
    ctx->device = device;

    lv_cf = px_to_lv_cf(ctx->lv_fmt);
    lv_cf_size = lv_color_format_get_size(lv_cf);
    if (lv_cf_size == 0)
    {
        lv_cf_size = (rt_uint32_t)(px_fmt_bpp(ctx->lv_fmt) / 8);
    }

#if defined(LVGL_DISP_REFRESH_PARTIAL)
    {
        rt_uint32_t lines = LVGL_DISP_PARTIAL_LINES;
        if (lines > info->height)
        {
            lines = info->height;
        }
        ctx->draw_buf_size = info->width * lines * lv_cf_size;
    }
#else
    ctx->draw_buf_size = info->width * info->height * lv_cf_size;
#endif

    ctx->draw_buf = rt_malloc(ctx->draw_buf_size);
    if (!ctx->draw_buf)
    {
        LOG_E("malloc draw buf %u fail", ctx->draw_buf_size);
        ret = -RT_ENOMEM;
        goto __fail;
    }

    display = lv_display_create(info->width, info->height);
    if (display == RT_NULL)
    {
        LOG_E("lvgl display create failed.");
        ret = -RT_ENOMEM;
        goto __fail;
    }

    lv_display_set_driver_data(display, device);
    lv_display_set_user_data(display, ctx);
    lv_display_set_color_format(display, lv_cf);

#if defined(LVGL_DISP_REFRESH_PARTIAL)
    lv_display_set_buffers(display, ctx->draw_buf, RT_NULL,
        ctx->draw_buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
#else
    lv_display_set_buffers(display, ctx->draw_buf, RT_NULL,
        ctx->draw_buf_size, LV_DISPLAY_RENDER_MODE_FULL);
#endif

    lv_display_set_flush_cb(display, lvgl_flush_cb);

    return ret;

__fail:
    if (device)
    {
        rt_device_close(device);
    }

    if (ctx->draw_buf)
    {
        rt_free(ctx->draw_buf);
        ctx->draw_buf = RT_NULL;
    }

    if (display)
    {
        lv_display_delete(display);
    }

    return ret;
}
