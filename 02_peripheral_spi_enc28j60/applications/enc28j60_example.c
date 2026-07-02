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
#include "enc28j60.h"

#define SPI_BUS_NAME    "spi0"
#define SPI_DEVICE_NAME "spi01"
#define SPI_CS_PIN      15
#define SPI_INT_PIN     32
#define SPI_RST_PIN     33

static int enc28j60_example(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *spi_device;

    if (rt_device_find("e2") != RT_NULL)
    {
        rt_kprintf("[enc28j60] device e2 already exists! Can not register again.\n");
        return -RT_EBUSY;
    }

    /* ==== INT Pin Init ==== */
    rt_pin_mode(SPI_INT_PIN, PIN_MODE_INPUT);
    rt_pin_attach_irq(SPI_INT_PIN, PIN_IRQ_MODE_RISING_FALLING, enc28j60_isr, RT_NULL);
    rt_pin_irq_enable(SPI_INT_PIN, PIN_IRQ_ENABLE);

    /* ==== RST Pin Init ==== */
    rt_pin_mode(SPI_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SPI_RST_PIN, PIN_LOW);
    rt_thread_delay(10);
    rt_pin_write(SPI_RST_PIN, PIN_HIGH);
    rt_thread_delay(10);

    /* ==== SPI Device Init ==== */
    spi_device = (struct rt_spi_device *)rt_device_find(SPI_DEVICE_NAME);
    if (spi_device == RT_NULL)
    {
        spi_device = rt_malloc(sizeof(struct rt_spi_device));
        if (spi_device == RT_NULL)
        {
            rt_kprintf("[enc28j60] malloc dev failed\n");
            return -RT_ERROR;
        }

        ret = rt_spi_bus_attach_device_cspin(spi_device, SPI_DEVICE_NAME, SPI_BUS_NAME, SPI_CS_PIN, RT_NULL);
        if (ret != RT_EOK)
        {
            rt_kprintf("[enc28j60] mount spi bus failed\n");
            rt_free(spi_device);
            return -RT_ERROR;
        }
        rt_kprintf("[enc28j60] spi device attach ok\n");
    }

    /* ==== ENC28J60 Attach ==== */
    ret = enc28j60_attach(SPI_DEVICE_NAME);
    if (ret != RT_EOK)
    {
        rt_kprintf("[enc28j60] enc28j60 attach fail!\n");
        return ret;
    }

    rt_kprintf("[enc28j60] example init done!\n");
    rt_kprintf("[enc28j60] use command: ifconfig e2 192.168.1.2 192.168.1.1 255.255.255.0\n");
    rt_kprintf("[enc28j60] then ping: ping 192.168.1.11\n");

    return ret;
}

MSH_CMD_EXPORT(enc28j60_example, enc28j60 ethernet example);
