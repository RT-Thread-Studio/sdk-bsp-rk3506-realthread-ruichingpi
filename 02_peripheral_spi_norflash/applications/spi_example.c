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

#define DBG_TAG "example.spi"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define SPI_BUS_NAME    "ff120000.spi"
#define SPI_DEVICE_NAME "w25q"
#define SPI_CS_PIN      15

static rt_err_t spi_device_init(void)
{
    rt_err_t ret = RT_EOK;
    struct rt_spi_device *dev;

    struct rt_spi_configuration cfg = {
        .mode = RT_SPI_MODE_0 | RT_SPI_MSB,
        .data_width = 8,
        .max_hz = 400000,
    };

    if (rt_device_find(SPI_DEVICE_NAME) == RT_NULL)
    {
        dev = rt_malloc(sizeof(struct rt_spi_device));
        if (dev == RT_NULL)
        {
            LOG_E("malloc dev failed");
            ret = (-RT_ERROR);
            goto dev_err;
        }

        ret = rt_spi_bus_attach_device_cspin(
            dev, SPI_DEVICE_NAME, SPI_BUS_NAME, SPI_CS_PIN, RT_NULL);
        if (ret != RT_EOK)
        {
            LOG_E("mount spi bus failed");
            ret = (-RT_ERROR);
            goto attach_err;
        }
    }

    dev = (struct rt_spi_device *)rt_device_find(SPI_DEVICE_NAME);
    if (dev == RT_NULL)
    {
        LOG_E("not find device %s", SPI_DEVICE_NAME);
        ret = (-RT_ERROR);
        goto dev_err;
    }

    rt_spi_configure(dev, &cfg);

    return RT_EOK;
attach_err:
    rt_free(dev);
dev_err:
    return ret;
}

static int spi_transfer(void)
{
    struct rt_spi_device *spi_dev_w25q = RT_NULL;
    rt_uint8_t w25x_read_id[6] = { 0x90, 0x00, 0x00, 0x00, 0x00, 0x00 };

    rt_uint8_t id[6] = { 0 };

    if (spi_device_init() != RT_EOK)
    {
        return (-RT_ERROR);
    }

    spi_dev_w25q = (struct rt_spi_device *)rt_device_find(SPI_DEVICE_NAME);

    rt_spi_transfer(spi_dev_w25q, w25x_read_id, id, 6);
    LOG_I("use rt_spi_transfer() read w25q ID is: 0x%X%X", id[4], id[5]);

    return RT_EOK;
}
MSH_CMD_EXPORT(spi_transfer, spi transfer);
