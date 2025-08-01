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
#include <sitronix_ts.h>
#include <sitronix_ts_utility.h>

struct sitronix_ts_data *gts = NULL;

static int sitronix_ts_reset_device(struct sitronix_ts_data *ts_data)
{
    rt_pin_mode(ts_data->host_if->rst_gpio, PIN_MODE_OUTPUT);
    rt_pin_write(ts_data->host_if->rst_gpio, PIN_LOW);

    rt_hw_us_delay(20000);

    rt_pin_write(ts_data->host_if->rst_gpio, PIN_HIGH);
    rt_hw_us_delay(100000);

    return 0;
}

static int16_t pre_x[] = { -1 };
static int16_t pre_y[] = { -1 };
static int16_t pre_w[] = { -1 };
static rt_uint8_t s_tp_dowm[1];
static struct rt_touch_data *read_data;

static void sitronix_touch_up(void *buf, int8_t id)
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

static void sitronix_touch_down(
    void *buf, int8_t id, int16_t x, int16_t y, int16_t w)
{
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
    read_data[id].x_coordinate = x;
    read_data[id].y_coordinate = y;
    read_data[id].track_id = id;

    pre_x[id] = x; /* save last point */
    pre_y[id] = y;
    pre_w[id] = w;
}

static rt_size_t sitronix_read_point(
    struct rt_touch_device *touch, void *buf, rt_size_t touch_num)
{
    struct sitronix_ts_data *ts_data = (struct sitronix_ts_data *)touch;
    uint8_t read_len = (ts_data->ts_dev_info.max_touches * 7) + 5;
    uint16_t coordCkhsum = 0x5A; //since v01.12.01
    uint16_t recvCkhsum = 0x00;
    int8_t read_id = 0;
    int16_t input_x = 0;
    int16_t input_y = 0;
    int16_t input_w = 0;
    int ret;

    ret = sitronix_ts_reg_read(ts_data, TOUCH_INFO, ts_data->coord_buf, 4);
    if (ret < 0)
    {
        rt_kprintf("%s: Read finger touch error!(%d)\n", __func__, ret);
        goto exit_invalid_data;
    }

    if (!(ts_data->coord_buf[0] & (1 << 3)))
    {
        goto exit_invalid_data;
    }

    if (ts_data->coord_buf[0] & 0x80)
    {
        /* ESD check fail */
        rt_kprintf("Firmware RstChip = 1 , reset device\n");
        sitronix_ts_reset_device(ts_data);
    }

    ret = sitronix_ts_reg_read(
        ts_data, TOUCH_INFO + 4, &ts_data->coord_buf[4], read_len);
    if (ret < 0)
    {
        rt_kprintf("%s: Read finger touch error!(%d)\n", __func__, ret);
        goto exit_invalid_data;
    }

    if (ts_data->is_support_coord_chksum)
    {
        /* support coord checksum*/
        st_checksum_calculation(&coordCkhsum, ts_data->coord_buf, 4);
        if (ts_data->coord_buf[0] & 0x08)
        {
            st_checksum_calculation(&coordCkhsum, ts_data->coord_buf + 4,
                (ts_data->ts_dev_info.max_touches * 7));
        }

        recvCkhsum = ts_data->coord_buf[read_len - 1];
        coordCkhsum = coordCkhsum & 0xFF;

        if (coordCkhsum != recvCkhsum)
        {
            rt_kprintf(
                "coord checksum error: "
                "expect = 0x%02X, received = 0x%02X\n",
                coordCkhsum, recvCkhsum);
            goto exit_invalid_data;
        }
    }

    /* just one */
    if (ts_data->coord_buf[4] & (1 << 7))
    {
        ts_data->coord_buf[4] &= ~(1 << 7);

        read_id = 0;
        input_x = (ts_data->coord_buf[4] << 8) | (ts_data->coord_buf[5]);
        input_y = (ts_data->coord_buf[6] << 8) | (ts_data->coord_buf[7]);
        input_w = (ts_data->coord_buf[8]);

        sitronix_touch_down(buf, read_id, input_x, input_y, input_w);
    }
    else
    {
        if (pre_x[0] != -1)
        {
            sitronix_touch_up(buf, 0);
        }
    }

    return 1;

exit_invalid_data:
    return 0;
}

static rt_err_t sitronix_control(
    struct rt_touch_device *touch, int cmd, void *arg)
{
    struct sitronix_ts_data *ts;
    struct rt_touch_info *info;

    ts = rt_container_of(touch, struct sitronix_ts_data, device);
    if (!ts)
    {
        return -RT_EINVAL;
    }

    switch (cmd)
    {
    case RT_TOUCH_CTRL_GET_ID: break;
    case RT_TOUCH_CTRL_GET_INFO:
        info = arg;
        info->range_x = ts->ts_dev_info.x_res;
        info->range_y = ts->ts_dev_info.y_res;
        info->point_num = 1;
        break;
    default: break;
    }

    return RT_EOK;
}

const struct rt_touch_ops sitronix_ops = {
    .touch_readpoint = sitronix_read_point,
    .touch_control = sitronix_control,
};

static rt_err_t sitronix_ts_probe(struct rt_platform_device *pdev)
{
    const struct sitronix_ts_host_interface *host_if;
    struct sitronix_ts_data *ts_data;
    struct rt_touch_config cfg;
    rt_err_t ret;

    host_if = pdev->priv;

    ts_data = rt_calloc(1, sizeof(*ts_data));
    if (!ts_data)
    {
        rt_kprintf("%s: Alloc memory for ts_data failed!\n", __func__);
        return -RT_ENOMEM;
    }

    ts_data->host_if = host_if;
    gts = ts_data;

    sitronix_ts_reset_device(ts_data);

    sitronix_ts_get_device_status(ts_data);
    ret = sitronix_ts_get_device_info(ts_data);
    if (ret)
    {
        rt_kprintf("%s: Failed to get device info.\n", __func__);
        ret = -RT_EINVAL;
        goto err_return;
    }

    cfg.irq_pin.pin = ts_data->host_if->irq_gpio;
    cfg.irq_pin.mode = PIN_MODE_INPUT_PULLDOWN;

    /* register touch device */
    ts_data->device.info.type = RT_TOUCH_TYPE_CAPACITANCE;
    ts_data->device.info.vendor = RT_TOUCH_VENDOR_GT;
    rt_memcpy(&ts_data->device.config, &cfg, sizeof(struct rt_touch_config));
    ts_data->device.ops = &sitronix_ops;

    rt_hw_touch_register(
        &ts_data->device, "touch", RT_DEVICE_FLAG_INT_RX, RT_NULL);

    return 0;
err_return:
    rt_free(ts_data);
    return ret;
}

static struct rt_platform_driver sitronix_ts_driver = {
    .name = "sitronix_ts",
    .ids = RT_NULL,
    .probe = sitronix_ts_probe,
};

static int sitronix_ts_register(void)
{
    return rt_platform_driver_register(&sitronix_ts_driver);
}
INIT_DEVICE_EXPORT(sitronix_ts_register);
