/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RINGBLK_BUF_H_
#define __RINGBLK_BUF_H_

#include <kpi.h>
#include <rtconfig.h>
#include <rtdef.h>

/*
 * Introduction:
 * The rbb is the ring buffer which is composed with many blocks. It is different from the ring buffer.
 * The ring buffer is only composed with chars. The rbb put and get supported zero copies. So the rbb
 * is very suitable for put block and get block by a certain order. Such as DMA block transmit,
 * communicate frame send/recv, and so on.
 */

enum rt_rbb_status
{
    /* unused status when first initialize or after blk_free() */
    RT_RBB_BLK_UNUSED,
    /* initialized status after blk_alloc() */
    RT_RBB_BLK_INITED,
    /* put status after blk_put() */
    RT_RBB_BLK_PUT,
    /* get status after blk_get() */
    RT_RBB_BLK_GET,
};
typedef enum rt_rbb_status rt_rbb_status_t;

/**
 * the block of rbb
 */
struct rt_rbb_blk
{
    rt_rbb_status_t status : 8;
    /* less then 2^24 */
    rt_size_t size : 24;
    rt_uint8_t *buf;
    rt_slist_t list;
};
typedef struct rt_rbb_blk *rt_rbb_blk_t;

/**
 * Rbb block queue: the blocks (from block1->buf to blockn->buf) memory which on this queue is continuous.
 */
struct rt_rbb_blk_queue
{
    rt_rbb_blk_t blocks;
    rt_size_t blk_num;
};
typedef struct rt_rbb_blk_queue *rt_rbb_blk_queue_t;

/**
 * ring block buffer
 */
struct rt_rbb
{
    rt_uint8_t *buf;
    rt_size_t buf_size;
    /* all of blocks */
    rt_rbb_blk_t blk_set;
    rt_size_t blk_max_num;
    /* saved the initialized and put status blocks */
    rt_slist_t blk_list;
    /* point to tail node */
    rt_slist_t *tail;
    /* free node list */
    rt_slist_t free_list;
    struct rt_spinlock spinlock;
};
typedef struct rt_rbb *rt_rbb_t;

/* rbb (ring block buffer) API */
typedef void (*__kpi_rt_rbb_init)(rt_rbb_t rbb,
    rt_uint8_t *buf,
    rt_size_t buf_size,
    rt_rbb_blk_t block_set,
    rt_size_t blk_max_num);
typedef rt_size_t (*__kpi_rt_rbb_get_buf_size)(rt_rbb_t rbb);

#ifdef RT_USING_HEAP
typedef rt_rbb_t (*__kpi_rt_rbb_create)(
    rt_size_t buf_size, rt_size_t blk_max_num);
typedef void (*__kpi_rt_rbb_destroy)(rt_rbb_t rbb);
#endif

/* rbb block API */
typedef rt_rbb_blk_t (*__kpi_rt_rbb_blk_alloc)(
    rt_rbb_t rbb, rt_size_t blk_size);
typedef void (*__kpi_rt_rbb_blk_put)(rt_rbb_blk_t block);
typedef rt_rbb_blk_t (*__kpi_rt_rbb_blk_get)(rt_rbb_t rbb);
typedef rt_size_t (*__kpi_rt_rbb_blk_size)(rt_rbb_blk_t block);
typedef rt_uint8_t *(*__kpi_rt_rbb_blk_buf)(rt_rbb_blk_t block);
typedef void (*__kpi_rt_rbb_blk_free)(rt_rbb_t rbb, rt_rbb_blk_t block);

/* rbb block queue API */
typedef rt_size_t (*__kpi_rt_rbb_blk_queue_get)(
    rt_rbb_t rbb, rt_size_t queue_data_len, rt_rbb_blk_queue_t blk_queue);
typedef rt_size_t (*__kpi_rt_rbb_blk_queue_len)(rt_rbb_blk_queue_t blk_queue);
typedef rt_uint8_t *(*__kpi_rt_rbb_blk_queue_buf)(rt_rbb_blk_queue_t blk_queue);
typedef void (*__kpi_rt_rbb_blk_queue_free)(
    rt_rbb_t rbb, rt_rbb_blk_queue_t blk_queue);
typedef rt_size_t (*__kpi_rt_rbb_next_blk_queue_len)(rt_rbb_t rbb);

KPI_EXTERN(rt_rbb_init);
KPI_EXTERN(rt_rbb_get_buf_size);
KPI_EXTERN(rt_rbb_create);
KPI_EXTERN(rt_rbb_destroy);
KPI_EXTERN(rt_rbb_blk_alloc);
KPI_EXTERN(rt_rbb_blk_put);
KPI_EXTERN(rt_rbb_blk_get);
KPI_EXTERN(rt_rbb_blk_size);
KPI_EXTERN(rt_rbb_blk_buf);
KPI_EXTERN(rt_rbb_blk_free);
KPI_EXTERN(rt_rbb_blk_queue_get);
KPI_EXTERN(rt_rbb_blk_queue_len);
KPI_EXTERN(rt_rbb_blk_queue_buf);
KPI_EXTERN(rt_rbb_blk_queue_free);
KPI_EXTERN(rt_rbb_next_blk_queue_len);

#endif /* __RINGBLK_BUF_H_ */
