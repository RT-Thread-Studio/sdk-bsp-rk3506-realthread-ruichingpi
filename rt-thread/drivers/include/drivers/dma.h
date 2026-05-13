#ifndef __DMA_H__
#define __DMA_H__

#include <rtthread.h>
#include <drivers/ofw.h>
#include <drivers/misc.h>
#include <drivers/core/dm.h>
#include "kpi.h"
#include <bitmap.h>

struct rt_dma_chan;
struct rt_dma_controller_ops;

enum rt_dma_transfer_direction
{
    RT_DMA_MEM_TO_MEM,
    RT_DMA_MEM_TO_DEV,
    RT_DMA_DEV_TO_MEM,
    RT_DMA_DEV_TO_DEV,

    RT_DMA_DIR_MAX,
};

enum rt_dma_slave_buswidth
{
    RT_DMA_SLAVE_BUSWIDTH_UNDEFINED     = 0,
    RT_DMA_SLAVE_BUSWIDTH_1_BYTE        = 1,
    RT_DMA_SLAVE_BUSWIDTH_2_BYTES       = 2,
    RT_DMA_SLAVE_BUSWIDTH_3_BYTES       = 3,
    RT_DMA_SLAVE_BUSWIDTH_4_BYTES       = 4,
    RT_DMA_SLAVE_BUSWIDTH_8_BYTES       = 8,
    RT_DMA_SLAVE_BUSWIDTH_16_BYTES      = 16,
    RT_DMA_SLAVE_BUSWIDTH_32_BYTES      = 32,
    RT_DMA_SLAVE_BUSWIDTH_64_BYTES      = 64,
    RT_DMA_SLAVE_BUSWIDTH_128_BYTES     = 128,

    RT_DMA_SLAVE_BUSWIDTH_BYTES_MAX,
};

struct rt_dma_slave_config
{
    enum rt_dma_transfer_direction direction;
    enum rt_dma_slave_buswidth src_addr_width;
    enum rt_dma_slave_buswidth dst_addr_width;

    rt_ubase_t src_addr;
    rt_ubase_t dst_addr;

    rt_uint32_t src_maxburst;
    rt_uint32_t dst_maxburst;
    rt_uint32_t src_port_window_size;
    rt_uint32_t dst_port_window_size;
};

struct rt_dma_slave_transfer
{
    rt_ubase_t src_addr;
    rt_ubase_t dst_addr;

    void *buffer;
    rt_ubase_t dma_handle;
    rt_size_t buffer_len;
    rt_size_t period_len;
};

struct rt_dma_controller
{
    rt_list_t list;

    struct rt_device *dev;

    RT_BITMAP_DECLARE(dir_cap, RT_DMA_DIR_MAX);
    const struct rt_dma_controller_ops *ops;

    rt_list_t channels_nodes;
    struct rt_mutex mutex;
};

struct rt_dma_controller_ops
{
    struct rt_dma_chan *(*request_chan)(struct rt_dma_controller *ctrl,
            struct rt_device *slave, void *fw_data);
    rt_err_t (*release_chan)(struct rt_dma_chan *chan);

    rt_err_t (*start)(struct rt_dma_chan *chan);
    rt_err_t (*stop)(struct rt_dma_chan *chan);
    rt_err_t (*config)(struct rt_dma_chan *chan, struct rt_dma_slave_config *conf);

    rt_err_t (*prep_memcpy)(struct rt_dma_chan *chan,
            rt_ubase_t dma_addr_src, rt_ubase_t dma_addr_dst, rt_size_t len);

    rt_err_t (*prep_cyclic)(struct rt_dma_chan *chan,
            rt_ubase_t dma_buf_addr, rt_size_t buf_len, rt_size_t period_len,
            enum rt_dma_transfer_direction dir);

    rt_err_t (*prep_single)(struct rt_dma_chan *chan,
            rt_ubase_t dma_buf_addr, rt_size_t buf_len,
            enum rt_dma_transfer_direction dir);
};

struct rt_dma_chan
{
    const char *name;

    struct rt_dma_controller *ctrl;
    struct rt_device *slave;

    rt_list_t list;
    rt_err_t conf_err;
    rt_err_t prep_err;
    struct rt_dma_slave_config conf;
    struct rt_dma_slave_transfer transfer;
    rt_uint32_t slave_id;

    void (*callback)(struct rt_dma_chan *chan, rt_size_t size);

    void *priv;
};

typedef rt_err_t (*__kpi_rt_dma_chan_start)(struct rt_dma_chan *chan);
typedef rt_err_t (*__kpi_rt_dma_chan_stop)(struct rt_dma_chan *chan);
typedef rt_err_t (*__kpi_rt_dma_chan_config)(struct rt_dma_chan *chan, struct rt_dma_slave_config *conf);
typedef rt_err_t (*__kpi_rt_dma_prep_memcpy)(struct rt_dma_chan *chan, struct rt_dma_slave_transfer *transfer);
typedef struct rt_dma_chan *(*__kpi_rt_dma_chan_request)(struct rt_device *dev, const char *name);
typedef rt_err_t (*__kpi_rt_dma_chan_release)(struct rt_dma_chan *chan);

KPI_EXTERN(rt_dma_chan_start);
KPI_EXTERN(rt_dma_chan_stop);
KPI_EXTERN(rt_dma_chan_config);
KPI_EXTERN(rt_dma_prep_memcpy);
KPI_EXTERN(rt_dma_chan_request);
KPI_EXTERN(rt_dma_chan_release);

#endif
