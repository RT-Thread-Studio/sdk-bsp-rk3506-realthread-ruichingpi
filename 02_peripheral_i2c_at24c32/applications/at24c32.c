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
#include <stdlib.h>
#include <string.h>

#define DBG_TAG "example.at24c32"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define EE_TWR                  5
#define EE_TYPE                 AT24C32
#define AT24C32_ADDR            (0xA0 >> 1)
#define AT24C32_PAGE_BYTE       32
#define AT24C32_MAX_MEM_ADDRESS 4096
#define BUFFER_SIZE             32

struct at24c32_device
{
    struct rt_i2c_bus_device *i2c;
    rt_mutex_t lock;
};
typedef struct at24c32_device *at24c32_device_t;

static const rt_uint8_t TEST_BUFFER[BUFFER_SIZE] = "WELCOME TO RTT EEPROM DEMO";

static rt_err_t read_regs(at24c32_device_t dev, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;

    msgs.addr = AT24C32_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    if (rt_i2c_transfer(dev->i2c, &msgs, 1) == 1)
    {
        return RT_EOK;
    }

    return (-RT_ERROR);
}

static rt_uint8_t at24c32_read_one_byte(
    at24c32_device_t dev, rt_uint16_t read_addr)
{
    rt_uint8_t buf[2], temp;

    buf[0] = (rt_uint8_t)(read_addr >> 8);
    buf[1] = (rt_uint8_t)(read_addr);

    if (rt_i2c_master_send(dev->i2c, AT24C32_ADDR, 0, buf, 2) == 0)
    {
        return RT_ERROR;
    }

    read_regs(dev, 1, &temp);

    return temp;
}

static rt_err_t at24c32_write_one_byte(
    at24c32_device_t dev, rt_uint16_t write_addr, rt_uint8_t to_write)
{
    rt_uint8_t buf[3];

    buf[0] = (rt_uint8_t)(write_addr >> 8);
    buf[1] = (rt_uint8_t)(write_addr);
    buf[2] = to_write;

    if (rt_i2c_master_send(dev->i2c, AT24C32_ADDR, 0, buf, 3) == 3)
    {
        return RT_EOK;
    }

    return (-RT_ERROR);
}

static rt_err_t at24c32_check(at24c32_device_t dev)
{
    rt_uint8_t temp;

    if (dev == RT_NULL)
    {
        LOG_E("at24c32_check: dev is NULL");
        return (-RT_ERROR);
    }

    temp = at24c32_read_one_byte(dev, AT24C32_MAX_MEM_ADDRESS - 1);
    if (temp == 0x55)
    {
        return RT_EOK;
    }
    else
    {
        at24c32_write_one_byte(dev, AT24C32_MAX_MEM_ADDRESS - 1, 0x55);
        rt_thread_mdelay(EE_TWR);
        temp = at24c32_read_one_byte(dev, AT24C32_MAX_MEM_ADDRESS - 1);
        if (temp == 0x55)
        {
            return RT_EOK;
        }
    }

    return (-RT_ERROR);
}

static rt_err_t at24c32_read(at24c32_device_t dev,
    rt_uint32_t read_addr,
    rt_uint8_t *buf,
    rt_uint16_t to_write)
{
    rt_err_t result;

    if (dev == RT_NULL)
    {
        LOG_E("at24c32_read: dev is NULL");
        return (-RT_EINVAL);
    }

    if (((read_addr + to_write) > AT24C32_MAX_MEM_ADDRESS) || (to_write == 0))
    {
        return (-RT_ERROR);
    }

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        while (to_write)
        {
            *buf++ = at24c32_read_one_byte(dev, read_addr++);
            to_write--;
        }
    }
    else
    {
        LOG_E("The at24c32 could not respond  at this time. Please try again");
    }

    rt_mutex_release(dev->lock);

    return RT_EOK;
}

static rt_err_t at24c32_write(at24c32_device_t dev,
    rt_uint32_t write_addr,
    const rt_uint8_t *buf,
    rt_uint16_t to_write)
{
    rt_uint16_t i = 0;
    rt_err_t result;

    if (dev == RT_NULL)
    {
        LOG_E("at24c32_write: dev is NULL");
        return (-RT_ERROR);
    }

    if (((write_addr + to_write) > AT24C32_MAX_MEM_ADDRESS) || (to_write == 0))
    {
        return (-RT_ERROR);
    }

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        while (1)
        {
            if (at24c32_write_one_byte(dev, write_addr, buf[i]) == RT_EOK)
            {
                write_addr++;
            }

            rt_thread_mdelay(EE_TWR);

            if (++i == to_write)
            {
                break;
            }
        }
    }
    else
    {
        LOG_E("The at24c32 could not respond  at this time. Please try again");
    }

    rt_mutex_release(dev->lock);

    return RT_EOK;
}

static at24c32_device_t at24c32_init(const char *i2c_bus_name)
{
    at24c32_device_t dev;

    if (i2c_bus_name == RT_NULL)
    {
        return RT_NULL;
    }

    dev = rt_calloc(1, sizeof(struct at24c32_device));
    if (dev == RT_NULL)
    {
        LOG_E("can't allocate memory for at24c32 device on '%s'", i2c_bus_name);
        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("can't find at24c32 device on '%s'", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_at24c32", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("can't create mutex for at24c32 device on '%s'", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    return dev;
}

static void at24c32_deinit(at24c32_device_t dev)
{
    if (dev->i2c == RT_NULL)
    {
        LOG_E("at24c32_deinit: dev->i2c is NULL");
        return;
    }

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

static void at24c32_example(void)
{
    rt_err_t ret = RT_EOK;
    static at24c32_device_t dev = RT_NULL;
    rt_uint8_t testbuffer[BUFFER_SIZE] = { 0 };

    dev = at24c32_init("i2c0");
    if (dev == RT_NULL)
    {
        LOG_E("init at24c32 device failed");
        return;
    }

    ret = at24c32_check(dev);
    if (ret != RT_EOK)
    {
        LOG_E("check at24c32 device failed");
        goto __err;
    }

    ret = at24c32_write(dev, 0, TEST_BUFFER, BUFFER_SIZE);
    if (ret != RT_EOK)
    {
        LOG_E("write to at24c32 device failed");
        goto __err;
    }

    ret = at24c32_read(dev, 0, testbuffer, BUFFER_SIZE);
    if (ret != RT_EOK)
    {
        LOG_E("read from at24c32 device failed");
        goto __err;
    }

    if (!rt_memcmp(TEST_BUFFER, testbuffer, BUFFER_SIZE))
    {
        LOG_I("at24c32 write/read verify success");
    }
    else
    {
        LOG_E("at24c32 write/read verify failed");
    }

__err:
    at24c32_deinit(dev);
}
MSH_CMD_EXPORT(at24c32_example, test at24c32 eeprom read / write);
