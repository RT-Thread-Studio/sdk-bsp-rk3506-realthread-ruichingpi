/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef APPLICATIONS_JPEG_H_
#define APPLICATIONS_JPEG_H_

#include <fcntl.h>
#include <rtthread.h>
#include "../components/graphics/lvgl/src/libs/tjpgd/tjpgd.h"



#define MJPEG_SRC_WIDTH   640
#define MJPEG_SRC_HEIGHT  480
#define JD_WORKSPACE_SIZE 8192

struct jpeg_input_ctx
{
    const uint8_t *jpeg_data;
    size_t jpeg_size;
    size_t offset;
};

#ifdef __cplusplus
extern "C"
{
#endif

extern rt_uint8_t tjpgd_work[JD_WORKSPACE_SIZE];

extern unsigned char *g_img_buf;
extern int g_img_size;

extern uint8_t *dec_rgb_buf;
extern size_t dec_rgb_buf_size;
extern uint8_t *scale_buf;
extern size_t scale_buf_size;

int load_image(const char *path, int max_size);

unsigned char *get_image_data(void);

int get_image_size(void);

void rgb_nearest_scale(const uint8_t *src_rgb, int src_w, int src_h, uint8_t *dst_rgb, int dst_w, int dst_h);

size_t input_func(JDEC *jdec, uint8_t *buff, size_t ndata);

int output_func(JDEC *jdec, void *bitmap, JRECT *rect);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATIONS_JPEG_H_ */
