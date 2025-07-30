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

#define DBG_TAG "example.rs485"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define RS485_NAME "uart5"
#define RS485_RTSN 57 // GPIO1_D1

char send_buf[] = "hello RTThread! \n";
char recv_buf[512];
int rx_len;

static struct serial_configure config_uart =
{
    BAUD_RATE_115200,   /* 115200 bits/s */
    DATA_BITS_8,        /* 8 databits */
    STOP_BITS_1,        /* 1 stopbit */
    PARITY_NONE,        /* No parity  */
    BIT_ORDER_LSB,      /* LSB first sent */
    NRZ_NORMAL,         /* Normal mode */
    RT_SERIAL_RB_BUFSZ, /* Buffer size */
    0
};

void rs485_send(rt_device_t dev, char *data, int len)
{
    rt_pin_write(RS485_RTSN, 1);
    rt_device_write(dev, 0, data, len);
}

rt_err_t rs485_rx_callback(rt_device_t dev, rt_size_t size)
{
    rx_len = rt_device_read(dev, 0, recv_buf, sizeof(recv_buf));
    if (rx_len > 0)
    {
        LOG_I("RS485 recv %d bytes: %.*s", rx_len, rx_len, recv_buf);
    }

    return RT_EOK;
}

void rs485_thread(void *arg)
{
    rt_device_t dev = rt_device_find(RS485_NAME);

    rt_device_control(dev, RT_DEVICE_CTRL_CONFIG, &config_uart);
    rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(dev, rs485_rx_callback);

    rt_pin_mode(RS485_RTSN, PIN_MODE_OUTPUT);

    for (int i = 0; i < 10; i++)
    {
        rs485_send(dev, send_buf, sizeof(send_buf) - 1);
        rt_thread_mdelay(500);
    }

    rt_pin_write(RS485_RTSN, 0);

    while (1) { rt_thread_mdelay(1000); }
}

rt_err_t rs485_example(void)
{
    rt_thread_t t =
        rt_thread_create("rs485", rs485_thread, RT_NULL, 2048, 15, 10);
    if (t == RT_NULL)
    {
        return (-RT_ERROR);
    }

    rt_thread_startup(t);

    return RT_EOK;
}
MSH_CMD_EXPORT(rs485_example, rs485 example);
