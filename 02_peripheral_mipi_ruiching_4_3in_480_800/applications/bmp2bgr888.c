/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <bmp2bgr888.h>

/**
 * BMP Bitmap Information Header (40 bytes)
 *
 * @note  Corresponds to Windows BITMAPINFOHEADER structure
 * @note  Positive height indicates bottom-up DIB (origin lower-left)
 *        Negative height indicates top-down DIB (origin upper-left)
 */
struct bmp_info_header
{
    rt_uint32_t bi_size;     /* Size of this structure in bytes. Must be 40 */
    int bi_width;            /* Image width in pixels */
    int bi_height;           /* Image height in pixels */
    rt_uint16_t bi_planes;   /* Number of color planes. Must be 1 */
    rt_uint16_t bi_bitcount; /* Bits per pixel (bpp) */
} __attribute__((packed));

/**
 * BMP Bitmap File Header (14 bytes)
 *
 * @note  All fields use little-endian byte order
 * @note  Structure uses packed attribute to prevent alignment padding
 */
struct bmp_bitmap_file_header
{
    rt_uint16_t bf_type; /* Must be 0x4D42 ("BM") */
    rt_uint32_t bf_size; /* Size of the entire BMP file in bytes */
    rt_uint32_t bf_reserved;
    rt_uint32_t bf_offbits; /* Offset from file start to pixel data in bytes */
} __attribute__((packed));

/**
 * bmp_extract_bgr888 - Extract BGR888 pixel data from BMP file
 * @bmp_data:   Pointer to raw BMP file data
 * @img_buffer: Output buffer for BGR888 pixel data
 *
 * Description:
 * Parses Windows BMP file format and extracts raw BGR888 pixel data.
 *
 * Return:
 * - RT_EOK on success
 * - RT_EINVAL for invalid parameters
 * - RT_ENOSYS for unsupported BMP format
 */
rt_err_t bmp_extract_bgr888(const rt_uint8_t *bmp_data, rt_uint8_t *img_buffer)
{
    struct bmp_bitmap_file_header header;
    struct bmp_info_header info;
    const rt_uint8_t *pixel_data;
    rt_uint32_t row_size;
    rt_uint8_t *dst_row;
    int is_top_down;
    rt_err_t ret;
    int src_y;
    int x, y;

    if (!bmp_data)
    {
        ret = (-RT_EINVAL);
        rt_kprintf("[BMP] Error: Invalid parameter (expect non-null)\n");
        return ret;
    }

    rt_memcpy(&header, bmp_data, sizeof(struct bmp_bitmap_file_header));
    rt_memcpy(&info, bmp_data + sizeof(struct bmp_bitmap_file_header),
        sizeof(struct bmp_info_header));

    if (header.bf_type != 0x4d42)
    {
        ret = (-RT_EINVAL);
        rt_kprintf("[BMP] Invalid signature (got 0x%04X, expected 0x4D42)",
            header.bf_type);
        return ret;
    }

    if (info.bi_bitcount != 24)
    {
        ret = (-RT_ENOSYS);
        rt_kprintf("[BMP] Error: Only 24-bit format supported (got %d-bit)\n",
            info.bi_bitcount);
        return ret;
    }

    if (info.bi_height < 0)
    {
        info.bi_height = -info.bi_height;
        is_top_down = 1;
    }
    else
    {
        is_top_down = 0;
    }

    row_size = ((info.bi_width * info.bi_bitcount + 31) / 32) * 4;

    pixel_data = bmp_data + header.bf_offbits;

    for (y = 0; y < info.bi_height; y++)
    {
        src_y = is_top_down ? y : (info.bi_height - 1 - y);
        const rt_uint8_t *src_row = pixel_data + src_y * row_size;
        dst_row = img_buffer + y * info.bi_width * 3;

        for (x = 0; x < info.bi_width; x++)
        {
            if (info.bi_bitcount == 24)
            {
                dst_row[x * 3 + 0] = src_row[x * 3 + 0]; /* B */
                dst_row[x * 3 + 1] = src_row[x * 3 + 1]; /* G */
                dst_row[x * 3 + 2] = src_row[x * 3 + 2]; /* R */
            }
        }
    }

    return RT_EOK;
}
