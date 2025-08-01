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
#include <gt9xx.h>

struct goodix_ts_data
{
    struct rt_touch_device device;
    struct rt_i2c_bus_device *i2c;
    struct rt_device *dev;
    rt_uint32_t client_addr;
    rt_uint16_t abs_x_max;
    rt_uint16_t abs_y_max;
    rt_uint8_t max_touch_num;
    rt_uint8_t int_trigger_type;

    rt_uint8_t pnl_init_error;
    int gtp_cfg_len;

    rt_uint32_t product_id;
};

struct gtp_config_custom
{
    rt_uint32_t gtp_resolution_x;
    rt_uint32_t gtp_resolution_y;
    rt_uint32_t gtp_change_x2y;
    rt_uint32_t gtp_overturn_x;
    rt_uint32_t gtp_overturn_y;
    rt_uint32_t gtp_send_cfgs;
    rt_uint32_t gtp_int_tarigger;
    rt_uint32_t gtp_touch_wakeup;
};

struct gtp_config_custom *gtp_config;

static rt_uint32_t gtp_rst_gpio;
static rt_uint32_t gtp_int_gpio;

static rt_uint8_t config[GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH] = {
    GTP_REG_CONFIG_DATA >> 8, GTP_REG_CONFIG_DATA & 0xff
};

static void gtp_parse_dt(struct rt_ofw_node *np)
{
    struct rt_ofw_prop *prop;
    const fdt32_t *cell;
    rt_uint32_t value;

    prop = rt_ofw_get_prop(np, "goodix_irq_gpio", RT_NULL);
    if (prop)
    {
        cell = rt_ofw_prop_next_u32(prop, RT_NULL, &value);
        cell = rt_ofw_prop_next_u32(prop, cell, &gtp_int_gpio);
    }
    else
    {
        gtp_int_gpio = -1;
    }

    prop = rt_ofw_get_prop(np, "goodix_rst_gpio", RT_NULL);
    if (prop)
    {
        cell = rt_ofw_prop_next_u32(prop, RT_NULL, &value);
        cell = rt_ofw_prop_next_u32(prop, cell, &gtp_rst_gpio);
    }
    else
    {
        gtp_int_gpio = -1;
    }

    rt_ofw_prop_read_u32(
        np, "gtp_resolution_x", &(gtp_config->gtp_resolution_x));
    rt_ofw_prop_read_u32(
        np, "gtp_resolution_y", &(gtp_config->gtp_resolution_y));
    rt_ofw_prop_read_u32(np, "gtp_change_x2y", &(gtp_config->gtp_change_x2y));
    rt_ofw_prop_read_u32(np, "gtp_overturn_x", &(gtp_config->gtp_overturn_x));
    rt_ofw_prop_read_u32(np, "gtp_overturn_y", &(gtp_config->gtp_overturn_y));
    rt_ofw_prop_read_u32(np, "gtp_send_cfg", &(gtp_config->gtp_send_cfgs));
    rt_ofw_prop_read_u32(
        np, "gtp_int_tarigger", &(gtp_config->gtp_int_tarigger));
    rt_ofw_prop_read_u32(
        np, "gtp_touch_wakeup", &(gtp_config->gtp_touch_wakeup));
}

static void gtp_int_sync(int ms)
{
    rt_pin_write(gtp_int_gpio, PIN_LOW);
    rt_hw_us_delay(ms * 1000);
    rt_pin_mode(gtp_int_gpio, PIN_MODE_INPUT);
}

static void gtp_reset_guitar(struct goodix_ts_data *ts, int ms)
{
    rt_pin_mode(gtp_int_gpio, PIN_MODE_OUTPUT);
    rt_pin_mode(gtp_rst_gpio, PIN_MODE_OUTPUT);

    rt_pin_write(gtp_int_gpio, PIN_LOW);
    rt_pin_write(gtp_rst_gpio, PIN_LOW);
    rt_hw_us_delay(ms * 1000);

    if (ts->client_addr == 0x14)
    {
        rt_pin_write(gtp_int_gpio, PIN_HIGH);
    }
    else
    {
        rt_pin_write(gtp_int_gpio, PIN_LOW);
    }

    rt_hw_us_delay(101);
    rt_pin_write(gtp_rst_gpio, PIN_HIGH);

    rt_hw_us_delay(6000);
    gtp_int_sync(50);
}

static rt_err_t gtp_i2c_read(
    struct goodix_ts_data *ts, rt_uint8_t *buf, rt_uint32_t len)
{
    struct rt_i2c_msg msgs[2] = { 0 };
    rt_uint32_t retries = 0;
    rt_err_t ret;

    msgs[0].flags = RT_I2C_WR;
    msgs[0].addr = ts->client_addr;
    msgs[0].len = GTP_ADDR_LENGTH;
    msgs[0].buf = &buf[0];

    msgs[1].flags = RT_I2C_RD;
    msgs[1].addr = ts->client_addr;
    msgs[1].len = len - GTP_ADDR_LENGTH;
    msgs[1].buf = &buf[GTP_ADDR_LENGTH];

    while (retries < 5)
    {
        ret = rt_i2c_transfer(ts->i2c, msgs, 2);
        if (ret == 2)
        {
            break;
        }
        retries++;
    }

    if ((retries >= 5))
    {
        gt_dbg(
            "I2C Read: 0x%04X, %d bytes failed,"
            "errcode: %d! Process reset.\n",
            (((rt_uint16_t)(buf[0] << 8)) | buf[1]), len - 2, ret);

        ret = (-RT_ERROR);

        gtp_reset_guitar(ts, 10);
    }

    return ret;
}

static rt_err_t gtp_i2c_write(
    struct goodix_ts_data *ts, rt_uint8_t *buf, int len)
{
    rt_err_t ret = (-RT_ERROR);
    int retries = 0;
    struct rt_i2c_msg msg;

    msg.flags = RT_I2C_WR;
    msg.addr = ts->client_addr;
    msg.len = len;
    msg.buf = buf;

    while (retries < 5)
    {
        ret = rt_i2c_transfer(ts->i2c, &msg, 1);
        if (ret == 1)
        {
            break;
        }

        retries++;
    }

    if ((retries >= 5))
    {
        gt_dbg(
            "I2C Write: 0x%04X, %d bytes failed, errcode: %d!"
            "Process reset.\n",
            (((rt_uint16_t)(buf[0] << 8)) | buf[1]), len - 2, ret);

        gtp_reset_guitar(ts, 10);
    }
    return ret;
}

static rt_err_t gtp_request_io_port(struct goodix_ts_data *ts)
{
    rt_pin_mode(gtp_int_gpio, PIN_MODE_OUTPUT);
    rt_pin_mode(gtp_rst_gpio, PIN_MODE_OUTPUT);

    gtp_reset_guitar(ts, 20);

    return RT_EOK;
}

static rt_err_t gtp_i2c_test(struct goodix_ts_data *ts)
{
    rt_uint8_t test[3] = { GTP_REG_CONFIG_DATA >> 8,
        GTP_REG_CONFIG_DATA & 0xff };
    rt_uint8_t retry = 0;
    rt_uint8_t ret = -1;

    while (retry++ < 5)
    {
        ret = gtp_i2c_read(ts, test, 3);
        if (ret > 0)
        {
            return RT_EOK;
        }

        gt_dbg("GTP i2c test failed time %d.", retry);
        rt_hw_us_delay(10000);
    }

    return (-RT_ERROR);
}

static rt_err_t gtp_read_version(
    struct goodix_ts_data *ts, rt_uint16_t *version)
{
    rt_uint8_t buf[8] = { GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xff };
    rt_err_t ret = -RT_ERROR;

    ret = gtp_i2c_read(ts, buf, sizeof(buf));
    if (ret < 0)
    {
        gt_warn("GTP read version failed.\n");
        return ret;
    }

    if (version)
    {
        *version = (buf[7] << 8) | buf[6];
    }
    if (buf[5] == 0x00)
    {
        gt_dbg("IC Version: %c%c%c_%02x%02x\n", buf[2], buf[3], buf[4], buf[7],
            buf[6]);
    }
    else
    {
        gt_dbg("IC Version: %c%c%c%c_%02x%02x\n", buf[2], buf[3], buf[4],
            buf[5], buf[7], buf[6]);
    }

    ts->product_id = (buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24));

    return ret;
}

static rt_err_t gtp_i2c_read_dbl_check(
    struct goodix_ts_data *ts, rt_uint16_t addr, rt_uint8_t *rxbuf, int len)
{
    rt_uint8_t confirm_buf[16] = { 0 };
    rt_uint8_t buf[16] = { 0 };
    rt_uint8_t retry = 0;

    while (retry++ < 3)
    {
        rt_memset(buf, 0xAA, 16);
        buf[0] = (rt_uint8_t)(addr >> 8);
        buf[1] = (rt_uint8_t)(addr & 0xFF);
        gtp_i2c_read(ts, buf, len + 2);

        rt_memset(confirm_buf, 0xAB, 16);
        confirm_buf[0] = (rt_uint8_t)(addr >> 8);
        confirm_buf[1] = (rt_uint8_t)(addr & 0xFF);
        gtp_i2c_read(ts, confirm_buf, len + 2);

        if (!rt_memcmp(buf, confirm_buf, len + 2))
        {
            rt_memcpy(rxbuf, confirm_buf + 2, len);
            return RT_EOK;
        }
    }

    gt_dbg("I2C read 0x%04X, %d bytes, double check failed!\n", addr, len);

    return (-RT_ERROR);
}

static rt_err_t gtp_parse_dt_cfg(
    struct goodix_ts_data *ts, rt_uint8_t *cfg, int *cfg_len, rt_uint8_t sid)
{
    struct rt_ofw_node *np = ts->dev->ofw_node;
    struct rt_ofw_prop *prop;
    char cfg_name[18];

    rt_snprintf(cfg_name, sizeof(cfg_name), "goodix,cfg-group%d", sid);

    prop = rt_ofw_get_prop(np, cfg_name, cfg_len);
    if ((!prop) || (!prop->value) || (*cfg_len == 0) ||
        (*cfg_len > GTP_CONFIG_MAX_LENGTH))
    {
        return (-RT_ERROR); /* failed */
    }
    else
    {
        rt_memcpy(cfg, prop->value, *cfg_len);
        return 0;
    }
}

static rt_err_t gtp_send_cfg(struct goodix_ts_data *ts)
{
    rt_err_t ret = 2;
    int retry;

    if (gtp_config->gtp_send_cfgs == 1)
    {
        if (ts->pnl_init_error)
        {
            gt_dbg("Error occured in init_panel, no config sent.\n");
            return 0;
        }

        for (retry = 0; retry < 5; retry++)
        {
            ret = gtp_i2c_write(
                ts, config, GTP_CONFIG_MAX_LENGTH + GTP_ADDR_LENGTH);
            if (ret > 0)
            {
                break;
            }
        }
    }

    return ret;
}

static rt_err_t gtp_init_panel(struct goodix_ts_data *ts)
{
    rt_uint8_t opr_buf[16] = { 0 };
    rt_uint8_t flash_cfg_version;
    rt_uint8_t drv_cfg_version;
    rt_uint8_t check_sum;
    rt_uint8_t sensor_id;
    rt_err_t ret;
    int i;

    if (gtp_config->gtp_send_cfgs == 0)
    {
        ts->gtp_cfg_len = GTP_CONFIG_MAX_LENGTH;
        ret = gtp_i2c_read(ts, config, ts->gtp_cfg_len + GTP_ADDR_LENGTH);
        if (ret < 0)
        {
            gt_dbg(
                "Read Config Failed,"
                "Using Default Resolution & INT Trigger!\n");
            ts->abs_x_max = gtp_config->gtp_resolution_x;
            ts->abs_y_max = gtp_config->gtp_resolution_y;
            ts->int_trigger_type = gtp_config->gtp_int_tarigger;
        }
    }

    /* check firmware */
    ret = gtp_i2c_read_dbl_check(ts, 0x41E4, opr_buf, 1);
    if (RT_EOK == ret)
    {
        if (opr_buf[0] != 0xBE)
        {
            gt_dbg("Firmware error, no config sent!\n");
            return (-RT_ERROR);
        }
    }

    /* read sensor id */
    ret = gtp_i2c_read_dbl_check(ts, GTP_REG_SENSOR_ID, &sensor_id, 1);
    if (RT_EOK == ret)
    {
        if (sensor_id >= 0x06)
        {
            gt_dbg("Invalid sensor_id(0x%02X), No Config Sent!\n", sensor_id);
            ts->pnl_init_error = 1;
            return (-RT_ERROR);
        }
    }
    else
    {
        gt_dbg("Failed to get sensor_id, No config sent!\n");
        ts->pnl_init_error = 1;
        return (-RT_ERROR);
    }

    /* parse config data*/
    ret = gtp_parse_dt_cfg(
        ts, &config[GTP_ADDR_LENGTH], &ts->gtp_cfg_len, sensor_id);
    if (ret < 0)
    {
        gt_dbg("Can't to parse config data form device tree.\n");
        ts->pnl_init_error = 1;
        return (-RT_ERROR);
    }

    if (ts->gtp_cfg_len < GTP_CONFIG_MIN_LENGTH)
    {
        gt_dbg(
            "Config Group%d is INVALID CONFIG GROUP(Len: %d)!"
            "NO Config Sent!"
            "You need to check you header file CFG_GROUP section!\n",
            sensor_id, ts->gtp_cfg_len);
        ts->pnl_init_error = 1;

        return (-RT_ERROR);
    }

    ret = gtp_i2c_read_dbl_check(ts, GTP_REG_CONFIG_DATA, &opr_buf[0], 1);
    if (ret == RT_EOK)
    {
        gt_dbg(
            "Config Version: %d, 0x%02X;\n"
            "IC Config Version: %d, 0x%02X\n",
            config[GTP_ADDR_LENGTH], config[GTP_ADDR_LENGTH], opr_buf[0],
            opr_buf[0]);

        flash_cfg_version = opr_buf[0];
        drv_cfg_version = config[GTP_ADDR_LENGTH];

        config[GTP_ADDR_LENGTH] = 0x00;
        if (flash_cfg_version < 90 && flash_cfg_version > drv_cfg_version)
        {
            config[GTP_ADDR_LENGTH] = 0x00;
        }
    }
    else
    {
        gt_dbg("Failed to get ic config version!No config sent!\n");
        return (-RT_ERROR);
    }

    config[RESOLUTION_LOC] = (rt_uint8_t)gtp_config->gtp_resolution_x;
    config[RESOLUTION_LOC + 1] =
        (rt_uint8_t)(gtp_config->gtp_resolution_x >> 8);
    config[RESOLUTION_LOC + 2] = (rt_uint8_t)gtp_config->gtp_resolution_y;
    config[RESOLUTION_LOC + 3] =
        (rt_uint8_t)(gtp_config->gtp_resolution_y >> 8);

    if (gtp_config->gtp_int_tarigger == 0) /* RISING */
    {
        config[TRIGGER_LOC] &= ~(0x3);
    }
    else if (gtp_config->gtp_int_tarigger == 1) /* FALLING */
    {
        config[TRIGGER_LOC] &= ~(0x3);
        config[TRIGGER_LOC] |= 0x01;
    }

    check_sum = 0;
    for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
    {
        check_sum += config[i];
    }
    config[ts->gtp_cfg_len] = (~check_sum) + 1;

    if ((ts->abs_x_max == 0) && (ts->abs_y_max == 0))
    {
        ts->abs_x_max =
            (config[RESOLUTION_LOC + 1] << 8) + config[RESOLUTION_LOC];
        ts->abs_y_max =
            (config[RESOLUTION_LOC + 3] << 8) + config[RESOLUTION_LOC + 2];
        ts->int_trigger_type = (config[TRIGGER_LOC]) & 0x03;
    }

    if (gtp_config->gtp_send_cfgs == 1)
    {
        ret = gtp_send_cfg(ts);
        if (ret < 0)
        {
            gt_dbg("Send config error.\n");
        }

        if (flash_cfg_version < 90 && flash_cfg_version > drv_cfg_version)
        {
            check_sum = 0;
            config[GTP_ADDR_LENGTH] = drv_cfg_version;
            for (i = GTP_ADDR_LENGTH; i < ts->gtp_cfg_len; i++)
            {
                check_sum += config[i];
            }
            config[ts->gtp_cfg_len] = (~check_sum) + 1;
        }
    }

    rt_hw_us_delay(10000);

    return RT_EOK;
}

static void gtp_soft_unreset(struct goodix_ts_data *ts)
{
    rt_uint8_t buf[3];

    buf[0] = (rt_uint8_t)(GTP_REG_SLEEP >> 8);
    buf[1] = (rt_uint8_t)(GTP_REG_SLEEP & 0xFF);
    buf[2] = 0x0;

    if (gtp_i2c_write(ts, buf, 3) < 0)
    {
        gt_dbg("Soft reset failed\n");
    }
}

static int16_t pre_x[GTP_MAX_TOUCH] = { -1, -1, -1, -1, -1 };
static int16_t pre_y[GTP_MAX_TOUCH] = { -1, -1, -1, -1, -1 };
static int16_t pre_w[GTP_MAX_TOUCH] = { -1, -1, -1, -1, -1 };
static rt_uint8_t s_tp_dowm[GTP_MAX_TOUCH];
static struct rt_touch_data *read_data;

static void gt9xx_touch_up(void *buf, int8_t id)
{
    read_data = (struct rt_touch_data *)buf;

    if (s_tp_dowm[id] == 1)
    {
        s_tp_dowm[id] = 0;
        read_data[id].event = RT_TOUCH_EVENT_UP;
    }
    else
    {
        read_data[id].event = RT_TOUCH_EVENT_NONE;
    }

    read_data[id].timestamp = rt_touch_get_ts();
    read_data[id].width = pre_w[id];
    read_data[id].x_coordinate = pre_x[id];
    read_data[id].y_coordinate = pre_y[id];
    read_data[id].track_id = id;

    pre_x[id] = -1; /* last point is none */
    pre_y[id] = -1;
    pre_w[id] = -1;
}

static void gt9xx_touch_down(
    void *buf, int8_t id, int16_t x, int16_t y, int16_t w)
{
    int16_t temp;

    read_data = (struct rt_touch_data *)buf;

    if (s_tp_dowm[id] == 1)
    {
        read_data[id].event = RT_TOUCH_EVENT_MOVE;
    }
    else
    {
        read_data[id].event = RT_TOUCH_EVENT_DOWN;
        s_tp_dowm[id] = 1;
    }

    read_data[id].timestamp = rt_touch_get_ts();
    read_data[id].width = w;

    if (gtp_config->gtp_change_x2y == 1)
    {
        temp = x;
        x = y;
        y = temp;

        if (gtp_config->gtp_overturn_x == 1)
        {
            x = gtp_config->gtp_resolution_y - x;
        }

        if (gtp_config->gtp_overturn_y == 1)
        {
            y = gtp_config->gtp_resolution_x - y;
        }
    }
    else
    {
        if (gtp_config->gtp_overturn_x)
        {
            x = gtp_config->gtp_resolution_x - x;
        }

        if (gtp_config->gtp_overturn_y)
        {
            y = gtp_config->gtp_resolution_y - y;
        }
    }

    read_data[id].x_coordinate = x;
    read_data[id].y_coordinate = y;
    read_data[id].track_id = id;

    pre_x[id] = x; /* save last point */
    pre_y[id] = y;
    pre_w[id] = w;
}

static rt_size_t gt9xx_read_point(
    struct rt_touch_device *touch, void *buf, rt_size_t read_num)
{
    struct goodix_ts_data *ts = (struct goodix_ts_data *)touch;
    rt_uint8_t cmd[8 * GTP_MAX_TOUCH + GTP_ADDR_LENGTH];
    rt_uint8_t *read_buf = &cmd[GTP_ADDR_LENGTH];
    rt_uint8_t write_buf[1 + GTP_ADDR_LENGTH];
    int16_t input_x = 0;
    int16_t input_y = 0;
    int16_t input_w = 0;
    int8_t read_id = 0;
    rt_uint8_t touch_num;
    rt_uint8_t read_index;

    static rt_uint8_t pre_touch = 0;
    static int8_t pre_id[GTP_MAX_TOUCH] = { 0 };

    /* point status register */
    cmd[0] = (rt_uint8_t)((GTP_READ_COOR_ADDR >> 8) & 0xFF);
    cmd[1] = (rt_uint8_t)(GTP_READ_COOR_ADDR & 0xFF);

    if (gtp_i2c_read(ts, cmd, GTP_ADDR_LENGTH + 1) < 0)
    {
        gt_dbg("Read point failed\n");
        read_num = 0;
        goto exit_;
    }

    if (cmd[2] == 0) /* no data */
    {
        read_num = 0;
        goto exit_;
    }

    if ((cmd[2] & 0x80) == 0) /* data is not ready */
    {
        read_num = 0;
        goto exit_;
    }

    touch_num = cmd[2] & 0x0f; /* get point num */

    if (touch_num > GTP_MAX_TOUCH) /* point num is not correct */
    {
        read_num = 0;
        goto exit_;
    }

    cmd[0] = (rt_uint8_t)((GTP_REG_POINT1 >> 8) & 0xFF);
    cmd[1] = (rt_uint8_t)(GTP_REG_POINT1 & 0xFF);

    /* read point num is touch_num */
    if (gtp_i2c_read(ts, cmd, (read_num * 8) + GTP_ADDR_LENGTH) < 0)
    {
        gt_dbg("Read point failed\n");
        read_num = 0;
        goto exit_;
    }

    if (pre_touch > touch_num) /* point up */
    {
        for (read_index = 0; read_index < pre_touch; read_index++)
        {
            rt_uint8_t j;

            for (j = 0; j < touch_num; j++) /* this time touch num */
            {
                read_id = read_buf[j * 8] & 0x0F;

                if (pre_id[read_index] == read_id) /* this id is not free */
                {
                    break;
                }

                if (j >= touch_num - 1)
                {
                    rt_uint8_t up_id;
                    up_id = pre_id[read_index];
                    gt9xx_touch_up(buf, up_id);
                }
            }
        }
    }

    if (touch_num)
    {
        rt_uint8_t off_set;

        for (read_index = 0; read_index < touch_num; read_index++)
        {
            off_set = read_index * 8;
            read_id = read_buf[off_set] & 0x0f;
            pre_id[read_index] = read_id;
            input_x = read_buf[off_set + 1] | (read_buf[off_set + 2] << 8);
            input_y = read_buf[off_set + 3] | (read_buf[off_set + 4] << 8);
            input_w = read_buf[off_set + 5] | (read_buf[off_set + 6] << 8);

            gt9xx_touch_down(buf, read_id, input_x, input_y, input_w);
        }
    }
    else if (pre_touch)
    {
        for (read_index = 0; read_index < pre_touch; read_index++)
        {
            gt9xx_touch_up(buf, pre_id[read_index]);
        }
    }

    pre_touch = touch_num;

exit_:
    write_buf[0] = (rt_uint8_t)((GTP_READ_COOR_ADDR >> 8) & 0xFF);
    write_buf[1] = (rt_uint8_t)(GTP_READ_COOR_ADDR & 0xFF);
    write_buf[2] = 0x00;
    gtp_i2c_write(ts, write_buf, sizeof(write_buf));

    return read_num;
}

static void gtxx_get_product_id(struct goodix_ts_data *ts, rt_uint8_t *args)
{
    args[0] = ts->product_id & 0xff;
    args[1] = (ts->product_id >> 8) & 0xff;
    args[2] = (ts->product_id >> 16) & 0xff;
    args[3] = (ts->product_id >> 24) & 0xff;
}

static rt_err_t gtxx_get_info(
    struct goodix_ts_data *ts, struct rt_touch_info *info)
{
    rt_uint8_t out_info[GTP_ADDR_LENGTH + 8];
    rt_uint8_t out_len = GTP_ADDR_LENGTH + 8;

    out_info[0] = (rt_uint8_t)(GTP_REG_CONFIG_DATA >> 8);
    out_info[1] = (rt_uint8_t)(GTP_REG_CONFIG_DATA & 0xFF);

    if (gtp_i2c_read(ts, out_info, out_len) < 0)
    {
        gt_err("Read info failed\n");
        return (-RT_ERROR);
    }

    info->range_x = (out_info[4] << 8) | out_info[3];
    info->range_y = (out_info[6] << 8) | out_info[5];
    info->point_num = out_info[7] & 0x0f;

    return RT_EOK;
}

static rt_err_t gt9xx_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    struct goodix_ts_data *ts;
    ts = rt_container_of(touch, struct goodix_ts_data, device);
    if (!ts)
    {
        return (-RT_EINVAL);
    }

    switch (cmd)
    {
    case RT_TOUCH_CTRL_GET_ID: gtxx_get_product_id(ts, arg); break;
    case RT_TOUCH_CTRL_GET_INFO: gtxx_get_info(ts, arg); break;
    default: break;
    }

    return RT_EOK;
}

static struct rt_touch_ops gt9xxtouch_ops = {
    .touch_readpoint = gt9xx_read_point,
    .touch_control = gt9xx_control,
};

static rt_err_t gt9xx_probe(struct rt_platform_device *pdev)
{
    rt_err_t ret;
    rt_int32_t id;
    rt_uint16_t version_info;
    char bus_name[RT_NAME_MAX];
    struct rt_ofw_node *port;
    struct rt_touch_config cfg;
    struct goodix_ts_data *ts;
    struct rt_ofw_node *np = pdev->parent.ofw_node;

    ts = rt_calloc(1, sizeof(*ts));
    if (ts == RT_NULL)
    {
        gt_err("Alloc goodix data memory failed.\n");
        ret = -RT_ENOMEM;
        goto ts_err;
    }

    ts->dev = &pdev->parent;

    gtp_config = rt_malloc(sizeof(*gtp_config));
    if (gtp_config == RT_NULL)
    {
        gt_err("Alloc gtp config memory failed.\n");
        ret = -RT_ENOMEM;
        goto gtp_config_err;
    }

    ret = rt_ofw_prop_read_u32(np, "reg", &ts->client_addr);
    if (ret != RT_EOK)
    {
        gt_err("The 0x%x addr is invalid.\n", ts->client_addr);
        ret = -RT_ERROR;
        goto reg_err;
    }

    port = rt_ofw_get_parent(np);
    if (!port)
    {
        ret = -RT_ERROR;
        goto port_err;
    }

    id = rt_ofw_get_alias_id(port, "i2c");
    rt_snprintf(bus_name, RT_NAME_MAX, "i2c%d", id);
    ts->i2c = rt_i2c_bus_device_find(bus_name);
    if (!ts->i2c)
    {
        ret = -RT_ERROR;
        goto i2c_bus_err;
    }

    if (rt_device_open(&(ts->i2c->parent), RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        gt_err("Open %s device failed", bus_name);
        return -RT_ERROR;
    }

    if (np)
    {
        gtp_parse_dt(np);
    }

    gtp_request_io_port(ts);

    ret = gtp_i2c_test(ts);
    if (ret < 0)
    {
        gt_err("I2C communication failed.\n");
        ret = -RT_ERROR;
        goto i2c_err;
    }

    ret = gtp_read_version(ts, &version_info);
    if (ret < 0)
    {
        goto version_err;
    }

    ret = gtp_init_panel(ts);
    if (ret < 0)
    {
        ts->abs_x_max = gtp_config->gtp_resolution_x;
        ts->abs_y_max = gtp_config->gtp_resolution_y;
        ts->int_trigger_type = gtp_config->gtp_int_tarigger;
    }

    gtp_soft_unreset(ts);

    cfg.user_data = (rt_uint8_t *)&gtp_rst_gpio;
    cfg.irq_pin.pin = gtp_int_gpio;

    if (ts->int_trigger_type == 0)
    {
        cfg.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    }
    else
    {
        cfg.irq_pin.mode = PIN_MODE_INPUT_PULLDOWN;
    }

    /* register touch device */
    ts->device.info.type = RT_TOUCH_TYPE_CAPACITANCE;
    ts->device.info.vendor = RT_TOUCH_VENDOR_GT;
    rt_memcpy(&ts->device.config, &cfg, sizeof(struct rt_touch_config));
    ts->device.ops = &gt9xxtouch_ops;

    ret = rt_hw_touch_register(
        &ts->device, "touch", RT_DEVICE_FLAG_INT_RX, RT_NULL);

    return ret;

version_err:
i2c_bus_err:
i2c_err:
port_err:
reg_err:
    rt_free(gtp_config);
gtp_config_err:
    rt_free(ts);
ts_err:
    return ret;
}

static const struct rt_ofw_node_id gt_i2c_ofw_ids[] = {
    { .compatible = "goodix,gt9xx" }, { /* sentinel */ }
};

static struct rt_platform_driver gt9xx_driver = {
    .name = "gt9xx",
    .ids = gt_i2c_ofw_ids,
    .probe = gt9xx_probe,
};

static int gt9xx_register(void)
{
    return rt_platform_driver_register(&gt9xx_driver);
}
INIT_DEVICE_EXPORT(gt9xx_register);
