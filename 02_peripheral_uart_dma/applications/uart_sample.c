/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "rtthread.h"
#include "rtdevice.h"

#define UART_NAME                       "uart3"
#define UART_TID_NAME                   "uart_thread"
#define UART_DMA_POLL_NAME              "uart_dma_poll"
#define UART_SEM_NAME                   "uart_sem"
#define UART_DATA_NUM                   (1500)
#define UART_DMA_INT                    (0)     // DMA 中断模式
#define UART_DMA_POLL                   (1)     // DMA 轮询模式
#define RT_UART_CTRL_GET_TX_STATUS      (0x10)  // 获取串口发送状态
#define RT_UART_CTRL_GET_RX_STATUS      (0x11)  // 获取串口接收状态
#define RT_UART_CTRL_GET_TX_DMA_STATUS  (0x12)  // 获取串口发送 DMA 状态
#define RT_UART_CTRL_GET_RX_DMA_STATUS  (0x13)  // 获取串口接收 DMA 状态
#define RT_UART_CTRL_START_RX_DMA       (0x14)  // 启动串口接收 DMA
#define RT_UART_CTRL_TX_DMA_DONE        (0x15)  // DMA 发送完成
#define RT_UART_CTRL_RX_DMA_DONE        (0x16)  // DMA 接收完成

static rt_device_t _serial = RT_NULL;
static rt_sem_t _sem = RT_NULL;
static rt_thread_t _uart_tid = RT_NULL, _dma_poll_tid = RT_NULL;

static rt_err_t rx_call(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(_sem);

    return RT_EOK;
}

/**
 * @brief DMA 轮询接收线程
 * @param  parameter
 * @note  必须在单独线程中运行，且优先级应高于数据处理线程
 */
static void _uart_rx_dma_poll(void *parameter)
{
    rt_uint32_t dma_status = 0, status = 0;

    while (1)
    {
        /* 获取 RX 状态 */
        rt_device_control(_serial, RT_UART_CTRL_GET_RX_STATUS, &status);
        /* status 不为 0 表示有数据 */
        if (status != 0)
        {
            /* 启动 DMA 接收 */
            rt_device_control(_serial, RT_UART_CTRL_START_RX_DMA, RT_NULL);

            while (1)
            {
                /* 获取 DMA 状态 */
                rt_device_control(_serial, RT_UART_CTRL_GET_RX_DMA_STATUS,
                                    &dma_status);
                /* dma_status 不为 0 表示 DMA 接收完成 */
                if (dma_status != 0)
                {
                    break;
                }
            }

            /* DMA 接收完成需要调用 RT_UART_CTRL_RX_DMA_DONE */
            rt_device_control(_serial, RT_UART_CTRL_RX_DMA_DONE, RT_NULL);
            continue;
        }

        rt_thread_delay(1);
    }
}

static void _uart_run_thread(void *parameter)
{
    char out_str[UART_DATA_NUM];
    rt_ssize_t size = 0, total_size = 0;
    rt_uint32_t status = 0, index = 0;
    rt_uint32_t mode = (rt_uint32_t)parameter;

    /* 准备发送数据 */
    for (index = 0; index < UART_DATA_NUM - 3; index++)
    {
        out_str[index] = 'a' + (index % 26);
    }
    out_str[index] = '\r';
    out_str[index + 1] = '\n';
    out_str[index + 2] = '\0';

    /* status 不为 0 表示串口可以发送数据 */
    rt_device_control(_serial, RT_UART_CTRL_GET_TX_STATUS, &status);
    if (status != 0)
    {
        /* 发送数据 */
        rt_device_write(_serial, 0, out_str, strlen(out_str));
        /* 轮询模式下等待发送完成  */
        if (mode == UART_DMA_POLL)
        {
            while (1)
            {
                rt_device_control(_serial, RT_UART_CTRL_GET_TX_DMA_STATUS,
                                    &status);
                /* status 不为 0 表示 DMA 发送完成 */
                if (status != 0)
                {
                    break;
                }
            }
            /* DMA 发送完成需要调用 RT_UART_CTRL_TX_DMA_DONE */
            rt_device_control(_serial, RT_UART_CTRL_TX_DMA_DONE, RT_NULL);
        }
    }

    rt_kprintf("uart dma tx done\n");

    memset(out_str, 0, UART_DATA_NUM);

    while (1)
    {
        /* 中断模式下，等待释放信号量 */
        if (mode == UART_DMA_INT)
        {
            rt_sem_take(_sem, RT_WAITING_FOREVER);
        }

        /* 读取数据 */
        size = rt_device_read(_serial, 0, &out_str[total_size], UART_DATA_NUM);

        total_size += size;

        /* 数据帧尾为 '#' */
        if (total_size > 0 && out_str[total_size - 1] == '#')
        {
            out_str[total_size - 1] = '\0';
            rt_kprintf("size:%d\n%s\n", total_size, out_str);
            memset(out_str, 0, UART_DATA_NUM);
            total_size = 0;
        }

        rt_thread_mdelay(10);
    }
}

int uart_dma_test(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t mode = 0;
    struct serial_configure uart_cfg;

    uart_cfg.baud_rate = BAUD_RATE_115200;
    uart_cfg.data_bits = DATA_BITS_8;
    uart_cfg.stop_bits = STOP_BITS_1;
    uart_cfg.parity = PARITY_NONE;
    uart_cfg.invert = NRZ_NORMAL;
    uart_cfg.bufsz = UART_DATA_NUM;
    uart_cfg.flowcontrol = 0;

    if (argc > 1)
    {
        mode = atoi(argv[1]);
    }

    _serial = rt_device_find(UART_NAME);
    if (_serial == RT_NULL)
    {
        rt_kprintf("can't find %s\n", UART_NAME);
        goto exit;
    }

    if (_serial->ref_count != 0)
    {
        if (_uart_tid != RT_NULL)
        {
            rt_thread_delete(_uart_tid);
            _uart_tid = RT_NULL;
        }
        if (_dma_poll_tid != RT_NULL)
        {
            rt_thread_delete(_dma_poll_tid);
            _dma_poll_tid = RT_NULL;
        }
        if (_sem != RT_NULL)
        {
            rt_sem_delete(_sem);
            _sem = RT_NULL;
        }

        rt_device_close(_serial);
    }

    /* 中断模式需要注册接收回调，轮询不需要注册接收回调 */
    if (mode == UART_DMA_INT)
    {
        rt_device_set_rx_indicate(_serial, rx_call);
        if (_sem == RT_NULL)
        {
            _sem = rt_sem_create(UART_SEM_NAME, 0, RT_IPC_FLAG_FIFO);
        }
    }
    else
    {
        rt_device_set_rx_indicate(_serial, RT_NULL);
    }

    /* 需要先设置回调再 open */
    rt_device_open(_serial, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_DMA_TX |
                    RT_DEVICE_FLAG_DMA_RX);

    rt_device_control(_serial, RT_DEVICE_CTRL_CONFIG, &uart_cfg);

    /* DMA 轮询接收需要单独启用一个线程处理 DMA 接收的数据 */
    if (mode == UART_DMA_POLL)
    {
        _dma_poll_tid = rt_thread_create(UART_DMA_POLL_NAME, _uart_rx_dma_poll,
                                        RT_NULL, 1024, 15, 10);
        if (_dma_poll_tid == RT_NULL)
        {
            rt_kprintf("create dma poll thread failed\n");
            goto exit;
        }
        rt_thread_startup(_dma_poll_tid);
    }

    _uart_tid = rt_thread_create(UART_TID_NAME, _uart_run_thread, (void *)mode,
                                4096, 15, 10);
    if (_uart_tid == RT_NULL)
    {
        rt_kprintf("create thread failed\n");
        goto exit;
    }
    rt_thread_startup(_uart_tid);

    return ret;
exit:
    ret = -RT_ERROR;
    return ret;
}
MSH_CMD_EXPORT(uart_dma_test, uart dma test sample);
