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

#define DBG_TAG "example.i2c"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define HYM8563S_BASE 0x51

static struct rt_i2c_bus_device *i2c = RT_NULL;

static rt_err_t hym8563s_write_reg(
    struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = { 0 };
    struct rt_i2c_msg msg;

    buf[0] = reg;
    buf[1] = data;

    msg.addr = HYM8563S_BASE;
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = 2;

    if (rt_i2c_transfer(bus, &msg, 1) == 1)
    {
        return RT_EOK;
    }

    return (-RT_ERROR);
}

static rt_err_t hym8563s_read_reg(struct rt_i2c_bus_device *bus,
    rt_uint8_t reg,
    rt_uint8_t len,
    rt_uint8_t *buf)
{
    struct rt_i2c_msg msg[2] = { 0 };

    msg[0].addr = HYM8563S_BASE;
    msg[0].flags = RT_I2C_WR;
    msg[0].len = 1;
    msg[0].buf = &reg;

    msg[1].addr = HYM8563S_BASE;
    msg[1].flags = RT_I2C_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    if (rt_i2c_transfer(bus, msg, 2) == 2)
    {
        return RT_EOK;
    }

    return (-RT_ERROR);
}

static void i2c_example(void)
{
    rt_uint8_t rx_buff[7] = { 0 };
    rt_uint8_t count = 10;

    i2c = rt_i2c_bus_device_find("i2c0");
    if (i2c == RT_NULL)
    {
        LOG_E("i2c0 not found!");
        return;
    }

    hym8563s_write_reg(i2c, 0x01, 0xA);

    hym8563s_write_reg(i2c, 0x02, 0x56); /* set second */
    hym8563s_write_reg(i2c, 0x03, 0x59); /* set minute */
    hym8563s_write_reg(i2c, 0x04, 0x23); /* set hour */
    hym8563s_write_reg(i2c, 0x05, 0x31); /* set day */
    hym8563s_write_reg(i2c, 0x07, 0x12); /* set mouth */
    hym8563s_write_reg(i2c, 0x08, 0x25); /* set year */
    hym8563s_write_reg(i2c, 0x06, 0x1);  /* set week */

    hym8563s_write_reg(i2c, 0x01, 0xA);

    while (count--)
    {
        hym8563s_read_reg(i2c, 0x02, 7, rx_buff); /* read time */
        LOG_I("20%02x--%02x--%02x  %02x : %02x : %02x  week: %02x", rx_buff[6],
            rx_buff[5] & 0x1f, rx_buff[3] & 0x3f, rx_buff[2] & 0x3f,
            rx_buff[1] & 0x7f, rx_buff[0] & 0x7f, rx_buff[4] & 0x07);

        rt_thread_mdelay(1000);
    }
}
MSH_CMD_EXPORT(i2c_example, i2c_read_rtc);
