/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef APPLICATIONS_UVC_BUFFER_H_
#define APPLICATIONS_UVC_BUFFER_H_
struct rt_uvc_resolution
{
    uint16_t width;
    uint16_t height;
};

struct uvc_raw_packet {
    uint8_t  *raw_buffer;
    uint32_t  raw_len;
    uint32_t   header_len;
    uint8_t  *payload;
    uint32_t  payload_len;
    uint32_t  frame_offset;
    uint8_t   eof;
    uint8_t   error;
};

struct raw_frame
{
    uint8_t *buf;
    uint32_t buf_size;
    uint32_t frame_len;
    volatile int ready;
    volatile uint32_t frame_size;
};


int raw_frame_init(struct raw_frame *frame, uint32_t size);
void raw_frame_deinit(struct raw_frame *frame);
int raw_double_buf_init(uint32_t size);
void raw_double_buf_deinit(void);
int save_one_frame(struct raw_frame *frame, uint8_t type);
extern struct raw_frame g_raw_frame[2];
extern volatile int g_fill_idx;
extern volatile int g_save_idx;
extern char doc_path[50];

#endif /* APPLICATIONS_UVC_BUFFER_H_ */
