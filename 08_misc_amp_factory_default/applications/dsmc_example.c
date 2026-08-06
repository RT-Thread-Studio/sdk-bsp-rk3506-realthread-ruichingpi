/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtdevice.h>
#include <drivers/dma.h>

#define DSMC_DMA_EVENT_DONE      (1 << 0)
#define DSMC_DMA_DEV_BASE        (0xC0000000U)
#define DSMC_DMA_WORD_NUM        (2 * 1024) //DMA访问DSMC单次最大传输8KB
#define DSMC_DMA_TEST_ROUNDS     (1000)
#define DSMC_DMA_CACHE_ALIGN     (256)
#define readl(reg)              (*((volatile rt_uint32_t *)(reg)))
#define writel(data, reg)       ((*((volatile rt_uint32_t *)(reg))) = (rt_uint32_t)(data))

static rt_event_t dsmc_dma_event = RT_NULL;

static void dsmc_dma_callback(struct rt_dma_chan *chan, rt_size_t size)
{
    (void)chan;
    (void)size;

    if (dsmc_dma_event != RT_NULL)
    {
        rt_event_send(dsmc_dma_event, DSMC_DMA_EVENT_DONE);
    }
}

static struct rt_dma_chan *dsmc_dma_request_chan(void)
{
    rt_device_t dev = rt_device_find("dsmc");

    if (dev == RT_NULL)
    {
        rt_kprintf("can't find dsmc device\n");
        return RT_NULL;
    }

    return rt_dma_chan_request(dev, RT_NULL);
}

static rt_err_t dsmc_dma_memcpy(struct rt_dma_chan *chan,
                                rt_ubase_t dst,
                                rt_ubase_t src,
                                rt_size_t len)
{
    rt_err_t ret;
    rt_uint32_t event;
    struct rt_dma_slave_transfer trans = {0};

    trans.src_addr = src;
    trans.dst_addr = dst;
    trans.buffer_len = len;

    ret = rt_dma_prep_memcpy(chan, &trans);
    if (ret != RT_EOK)
    {
        rt_kprintf("DMA prepare memcpy failed: %d\n", ret);
        return ret;
    }

    ret = rt_dma_chan_start(chan);
    if (ret != RT_EOK)
    {
        rt_kprintf("DMA start failed: %d\n", ret);
        return ret;
    }

    ret = rt_event_recv(dsmc_dma_event, DSMC_DMA_EVENT_DONE,
                        RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                        10000, &event);
    if (ret != RT_EOK)
    {
        rt_kprintf("wait DMA event failed: %d\n", ret);
    }

    return ret;
}

static rt_err_t dsmc_dma_init(struct rt_dma_chan **chan)
{
    rt_err_t ret;
    struct rt_dma_slave_config cfg = {0};

    *chan = dsmc_dma_request_chan();
    if (rt_is_err_or_null(*chan))
    {
        rt_kprintf("request DMA channel failed: %p\n", *chan);
        *chan = RT_NULL;
        return -RT_ERROR;
    }
    (*chan)->callback = dsmc_dma_callback;

    dsmc_dma_event = rt_event_create("dsmc_dma", RT_IPC_FLAG_FIFO);
    if (dsmc_dma_event == RT_NULL)
    {
        rt_kprintf("create DSMC DMA event failed\n");
        rt_dma_chan_release(*chan);
        *chan = RT_NULL;
        return -RT_ENOMEM;
    }

    cfg.direction = RT_DMA_MEM_TO_MEM;
    cfg.src_addr = 0;
    cfg.dst_addr = 0;
    cfg.src_addr_width = 8;
    cfg.dst_addr_width = 8;
    cfg.src_maxburst = 16;
    cfg.dst_maxburst = 16;
    ret = rt_dma_chan_config(*chan, &cfg);
    if (ret != RT_EOK)
    {
        rt_kprintf("configure DMA channel failed: %d\n", ret);
        rt_event_delete(dsmc_dma_event);
        dsmc_dma_event = RT_NULL;
        rt_dma_chan_release(*chan);
        *chan = RT_NULL;
    }

    return ret;
}

static void dsmc_dma_deinit(struct rt_dma_chan *chan)
{
    if (dsmc_dma_event != RT_NULL)
    {
        rt_event_delete(dsmc_dma_event);
        dsmc_dma_event = RT_NULL;
    }
    if (chan != RT_NULL)
    {
        rt_dma_chan_release(chan);
    }
}


static int dsmc_dma_write_dma_read(void)
{
    int i, j;
    int err_count = 0;
    int rounds_done = 0;
    rt_err_t ret = RT_EOK;
    rt_uint64_t t_start, t_end;
    rt_uint64_t dma_write_cycles, dma_read_cycles;
    rt_uint64_t dma_write_ns, dma_read_ns;
    rt_uint64_t total_dma_write_cycles = 0, total_dma_read_cycles = 0;
    rt_uint64_t total_dma_write_ns = 0, total_dma_read_ns = 0;
    rt_uint64_t avg_dma_write_ns = 0, avg_dma_read_ns = 0;
    rt_uint64_t avg_dma_write_us = 0, avg_dma_read_us = 0;
    rt_uint64_t dma_write_speed_x100 = 0, dma_read_speed_x100 = 0;
    rt_uint64_t dma_write_speed_int = 0, dma_read_speed_int = 0;
    rt_uint64_t dma_write_speed_frac = 0, dma_read_speed_frac = 0;
    const rt_uint32_t total_bytes = DSMC_DMA_WORD_NUM * sizeof(rt_uint32_t);
    struct rt_dma_chan *chan = RT_NULL;
    rt_uint32_t *write_buf = RT_NULL;
    rt_uint32_t *read_buf = RT_NULL;

    write_buf = rt_malloc_align(total_bytes, DSMC_DMA_CACHE_ALIGN);
    read_buf = rt_malloc_align(total_bytes, DSMC_DMA_CACHE_ALIGN);
    if (write_buf == RT_NULL || read_buf == RT_NULL)
    {
        rt_kprintf("DMA buffer malloc failed!\n");
        ret = -RT_ENOMEM;
        goto __exit;
    }

    ret = dsmc_dma_init(&chan);
    if (ret != RT_EOK)
    {
        goto __exit;
    }

    for (i = 0; i < DSMC_DMA_WORD_NUM; i++)
    {
        write_buf[i] = 0x12345678 + i * rt_hw_global_timer_get();
    }

    for (j = 0; j < DSMC_DMA_TEST_ROUNDS; j++)
    {
        rt_memset(read_buf, 0, total_bytes);
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, write_buf, total_bytes);
        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, read_buf, total_bytes);

        t_start = rt_hw_global_timer_get();
        ret = dsmc_dma_memcpy(chan, DSMC_DMA_DEV_BASE, (rt_ubase_t)write_buf, total_bytes);
        t_end = rt_hw_global_timer_get();
        if (ret != RT_EOK)
        {
            break;
        }
        dma_write_cycles = t_end - t_start;
        dma_write_ns = (dma_write_cycles * 1000ULL) / 24ULL;
        total_dma_write_cycles += dma_write_cycles;
        total_dma_write_ns += dma_write_ns;

        rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, read_buf, total_bytes);

        t_start = rt_hw_global_timer_get();
        ret = dsmc_dma_memcpy(chan, (rt_ubase_t)read_buf, DSMC_DMA_DEV_BASE, total_bytes);
        t_end = rt_hw_global_timer_get();
        if (ret != RT_EOK)
        {
            break;
        }
        rt_thread_mdelay(5);
        dma_read_cycles = t_end - t_start;
        dma_read_ns = (dma_read_cycles * 1000ULL) / 24ULL;
        total_dma_read_cycles += dma_read_cycles;
        total_dma_read_ns += dma_read_ns;
        rounds_done++;

        rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, read_buf, total_bytes);

        for (i = 0; i < DSMC_DMA_WORD_NUM; i++)
        {
            if (read_buf[i] != write_buf[i])
            {
                err_count++;
                rt_kprintf("DMA-W DMA-R ERR[round=%d off=0x%x]: R=0x%08X W=0x%08X\n",
                           j, i * (int)sizeof(rt_uint32_t), read_buf[i], write_buf[i]);
            }
            if (err_count > 10)
            {
                break;
            }
        }
        if (err_count > 10)
        {
            break;
        }
    }

    if (rounds_done == 0)
    {
        rounds_done = 1;
    }

    avg_dma_write_ns = total_dma_write_ns / (rt_uint64_t)rounds_done;
    avg_dma_read_ns = total_dma_read_ns / (rt_uint64_t)rounds_done;
    avg_dma_write_us = avg_dma_write_ns / 1000ULL;
    avg_dma_read_us = avg_dma_read_ns / 1000ULL;

    if (total_dma_write_cycles)
    {
        dma_write_speed_x100 = ((rt_uint64_t)total_bytes * (rt_uint64_t)rounds_done * 100ULL * 24ULL * 1000ULL * 1000ULL) /
                               (total_dma_write_cycles * 1024ULL * 1024ULL);
    }
    if (total_dma_read_cycles)
    {
        dma_read_speed_x100 = ((rt_uint64_t)total_bytes * (rt_uint64_t)rounds_done * 100ULL * 24ULL * 1000ULL * 1000ULL) /
                              (total_dma_read_cycles * 1024ULL * 1024ULL);
    }

    dma_write_speed_int = dma_write_speed_x100 / 100ULL;
    dma_read_speed_int = dma_read_speed_x100 / 100ULL;
    dma_write_speed_frac = dma_write_speed_x100 % 100ULL;
    dma_read_speed_frac = dma_read_speed_x100 % 100ULL;

    if (ret == RT_EOK && err_count == 0)
    {
        rt_kprintf("\n==== DSMC DMA WRITE + DMA READ PASS ====\n");
        rt_kprintf("Rounds: %d\n", rounds_done);
        rt_kprintf("DMA Write(avg): %llu ns, %llu us, %llu.%02llu MB/s\n",
                   avg_dma_write_ns, avg_dma_write_us, dma_write_speed_int, dma_write_speed_frac);
        rt_kprintf("DMA Read (avg): %llu ns, %llu us, %llu.%02llu MB/s\n",
                   avg_dma_read_ns, avg_dma_read_us, dma_read_speed_int, dma_read_speed_frac);
    }
    else
    {
        rt_kprintf("\n==== DSMC DMA WRITE + DMA READ FAIL ====\n");
        rt_kprintf("ret=%d, Error count: %d\n", ret, err_count);
    }

__exit:
    dsmc_dma_deinit(chan);
    if (write_buf != RT_NULL)
    {
        rt_free_align(write_buf);
    }
    if (read_buf != RT_NULL)
    {
        rt_free_align(read_buf);
    }

    return (ret == RT_EOK) ? err_count : ret;
}
MSH_CMD_EXPORT(dsmc_dma_write_dma_read, dsmc dma write and dma read test);