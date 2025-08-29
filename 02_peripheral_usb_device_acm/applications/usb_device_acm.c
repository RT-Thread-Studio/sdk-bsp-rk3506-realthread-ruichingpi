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

#define DBG_TAG "example.dev.acm"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define ACM_NAME "uart6"

#define RB_SIZE 2048

static char send_buf[] = "hello RTThread! \r\n";
static char read_buf[RB_SIZE];
static rt_sem_t acm_sem;
static int rx_len = 0;

static struct serial_configure config_uart =
{
    BAUD_RATE_115200,   /* 115200 bits/s */
    DATA_BITS_8,        /* 8 databits */
    STOP_BITS_1,        /* 1 stopbit */
    PARITY_NONE,        /* No parity  */
    BIT_ORDER_LSB,      /* LSB first sent */
    NRZ_NORMAL,         /* Normal mode */
    4096,               /* Buffer size */
    0
};

rt_err_t cdc_acm_rx_callback(rt_device_t dev, rt_size_t size)
{

    rx_len = rt_device_read(dev, 0, read_buf, RB_SIZE);
    if (rx_len > 0)
    {
        rt_sem_release(acm_sem);
    }

    return RT_EOK;
}

void cdc_acm_thread(void *arg)
{
    int ret = 0;

    acm_sem = rt_sem_create("acm", 0, RT_IPC_FLAG_PRIO);

    rt_device_t dev = rt_device_find(ACM_NAME);

    rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &config_uart);
    rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | \
        RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX);
    rt_device_set_rx_indicate(dev, cdc_acm_rx_callback);

    while(1)
    {
        rt_device_write(dev, 0, send_buf, sizeof(send_buf));

        ret = rt_sem_take(acm_sem, 500);
        if(ret ==RT_EOK)
        {
            LOG_I("ACM recv %d bytes: %.*s", rx_len, rx_len, read_buf);
        }
    }
}

rt_err_t cdc_acm_example(void)
{
    rt_thread_t t =
        rt_thread_create("cdc_acm", cdc_acm_thread, RT_NULL, 2048, 15, 10);
    if (t == RT_NULL)
    {
        return (-RT_ERROR);
    }

    rt_thread_startup(t);

    return RT_EOK;
}
MSH_CMD_EXPORT(cdc_acm_example, usb device cdc acm example);
