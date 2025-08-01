/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "hym8563s.h"

static rt_uint16_t hym8563s_bcd_to_dec(rt_uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

static rt_uint16_t hym8563s_dec_to_bcd(rt_uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

void hym8563s_get_time(struct rt_i2c_bus_device *i2c_dev, rt_uint32_t addr,
    time_t *r_time)
{
    rt_uint8_t rec_data[8] = {0x00};
    rt_uint8_t write_buf[1] = {0x02};
    struct tm time = {0};

    rt_i2c_master_send(i2c_dev, addr, RT_I2C_WR | RT_I2C_NO_STOP, write_buf, 1);
    rt_i2c_master_recv(i2c_dev, addr, RT_I2C_RD, rec_data, 7);

    if (rec_data != RT_NULL)
    {
        time.tm_sec = hym8563s_bcd_to_dec(rec_data[0] & 0x7f);
        time.tm_min = hym8563s_bcd_to_dec(rec_data[1] & 0x7f);
        time.tm_hour = hym8563s_bcd_to_dec(rec_data[2] & 0x3f);
        time.tm_mday = hym8563s_bcd_to_dec(rec_data[3] & 0x3f);
        time.tm_wday = hym8563s_bcd_to_dec(rec_data[4] & 0x07);
        time.tm_mon = hym8563s_bcd_to_dec(rec_data[5] & 0x1f) - 1;
        time.tm_year = hym8563s_bcd_to_dec(rec_data[6]) + 100;
        *r_time = mktime(&time);
    }
}

void hym8563s_set_time(struct rt_i2c_bus_device *i2c_dev, rt_uint32_t addr,
    time_t *r_time)
{
    rt_uint8_t write_buf[8] = {0};
    struct tm time = {0};

    time = *localtime(r_time);

    write_buf[0] = 0x02;
    write_buf[1] = hym8563s_dec_to_bcd(time.tm_sec);
    write_buf[2] = hym8563s_dec_to_bcd(time.tm_min);
    write_buf[3] = hym8563s_dec_to_bcd(time.tm_hour);
    write_buf[4] = hym8563s_dec_to_bcd(time.tm_mday);
    write_buf[5] = hym8563s_dec_to_bcd(time.tm_wday);
    write_buf[6] = hym8563s_dec_to_bcd(time.tm_mon + 1);

    if(time.tm_year > 100)
    {
        write_buf[7] = hym8563s_dec_to_bcd(time.tm_year - 100);
    }
    else
    {
        write_buf[7] = 100;
    }

    rt_i2c_master_send(i2c_dev, addr, RT_I2C_WR, write_buf, 8);
}
