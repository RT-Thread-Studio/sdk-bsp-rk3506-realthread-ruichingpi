/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "tsc2046.h"

#define DBG_TAG "tsc2046"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_err_t tsc2046_power_down(struct tsc2046_data *ts)
{
    rt_uint8_t tx[3] = {TSC2046_CMD_POWER_DOWN, 0, 0};
    rt_uint8_t rx[3];

    ts->last_spi_len = rt_spi_transfer(ts->spi, tx, rx, sizeof(tx));
    if (ts->last_spi_len != sizeof(tx))
    {
        ts->spi_errors++;
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_int32_t tsc2046_filter_axis(const rt_uint8_t *rx_buf,
                                      rt_size_t offset,
                                      rt_uint32_t max_spread)
{
    rt_uint16_t values[TSC2046_FILTER_SAMPLES];
    rt_uint16_t value;
    rt_size_t sample_offset;
    rt_uint32_t best_start = 0;
    rt_uint32_t best_spread;
    rt_uint32_t spread;
    rt_uint32_t i, j;

    for (i = 0; i < TSC2046_FILTER_SAMPLES; i++)
    {
        sample_offset = offset + i * TSC2046_TRANSFER_BYTES;
        value = ((((rt_uint16_t)rx_buf[sample_offset + 1] << 8) |
                  rx_buf[sample_offset + 2]) >> 3) & TSC2046_MAX_12BIT;

        for (j = i; j > 0 && values[j - 1] > value; j--)
        {
            values[j] = values[j - 1];
        }
        values[j] = value;
    }

    best_spread = values[2] - values[0];
    for (i = 1; i <= TSC2046_FILTER_SAMPLES - 3; i++)
    {
        spread = values[i + 2] - values[i];
        if (spread < best_spread)
        {
            best_spread = spread;
            best_start = i;
        }
    }

    if (max_spread && best_spread > max_spread)
    {
        return -RT_ERROR;
    }

    return (values[best_start] + values[best_start + 1] +
            values[best_start + 2] + 1) / 3;
}

static rt_err_t tsc2046_prepare_transfer(struct tsc2046_data *ts)
{
    rt_uint64_t settle_clocks;
    rt_uint64_t transfer_count;

    settle_clocks = (rt_uint64_t)ts->settle_delay_usecs *
                    ts->spi->config.max_hz;
    ts->settle_samples = (rt_uint32_t)((settle_clocks + 24000000 - 1) /
                                       24000000);
    if (ts->settle_samples > TSC2046_MAX_SETTLE_SAMPLES)
    {
        return -RT_EINVAL;
    }

    transfer_count = (rt_uint64_t)TSC2046_CHANNEL_COUNT *
                     (ts->settle_samples + TSC2046_FILTER_SAMPLES) + 1;
    ts->transfer_len = (rt_size_t)(transfer_count * TSC2046_TRANSFER_BYTES);
    ts->tx_buf = rt_calloc(1, ts->transfer_len);
    ts->rx_buf = rt_calloc(1, ts->transfer_len);
    if (ts->tx_buf == RT_NULL || ts->rx_buf == RT_NULL)
    {
        rt_free(ts->tx_buf);
        rt_free(ts->rx_buf);
        ts->tx_buf = RT_NULL;
        ts->rx_buf = RT_NULL;
        return -RT_ENOMEM;
    }

    return RT_EOK;
}

static rt_err_t tsc2046_read_state(struct tsc2046_data *ts,
                                   struct tsc2046_sample *sample)
{
    rt_uint64_t pressure;
    rt_size_t offset;
    rt_ssize_t len;
    rt_uint32_t axis;
    rt_uint32_t i;
    rt_uint8_t commands[TSC2046_CHANNEL_COUNT] =
    {
        TSC2046_CMD_X,
        TSC2046_CMD_Y,
        TSC2046_CMD_Z1,
        TSC2046_CMD_Z2,
    };
    rt_int32_t *samples[TSC2046_CHANNEL_COUNT] =
    {
        &sample->x,
        &sample->y,
        &sample->z1,
        &sample->z2,
    };

    rt_memset(ts->tx_buf, 0, ts->transfer_len);
    offset = 0;
    for (axis = 0; axis < TSC2046_CHANNEL_COUNT; axis++)
    {
        if (ts->keep_vref_on)
        {
            commands[axis] |= 0x02U;
        }

        for (i = 0; i < ts->settle_samples + TSC2046_FILTER_SAMPLES; i++)
        {
            ts->tx_buf[offset] = commands[axis];
            offset += TSC2046_TRANSFER_BYTES;
        }
    }
    ts->tx_buf[offset] = TSC2046_CMD_POWER_DOWN;

    len = rt_spi_transfer(ts->spi, ts->tx_buf, ts->rx_buf,
                          ts->transfer_len);
    ts->last_spi_len = len;
    if (len != (rt_ssize_t)ts->transfer_len)
    {
        ts->spi_errors++;
        ts->initialized = tsc2046_power_down(ts) == RT_EOK;
        return -RT_ERROR;
    }
    ts->initialized = RT_TRUE;

    offset = 0;
    for (axis = 0; axis < TSC2046_CHANNEL_COUNT; axis++)
    {
        *samples[axis] = tsc2046_filter_axis(
            ts->rx_buf, offset + ts->settle_samples * TSC2046_TRANSFER_BYTES,
            axis < 2 ? TSC2046_FILTER_MAX_SPREAD : 0);
        if (*samples[axis] < 0)
        {
            return -RT_ERROR;
        }
        offset += (ts->settle_samples + TSC2046_FILTER_SAMPLES) *
                  TSC2046_TRANSFER_BYTES;
    }

    if (sample->x > 0 && sample->z1 > 0 && sample->z2 > sample->z1)
    {
        pressure = (rt_uint64_t)(sample->z2 - sample->z1) *
                   ts->x_plate_ohms;
        pressure = (pressure + 8) / 16;
        pressure *= sample->x;
        pressure = (pressure + sample->z1 / 2) / sample->z1;
        sample->pressure = (rt_uint32_t)((pressure + 128) / 256);
    }
    else
    {
        sample->pressure = 0;
    }

    return RT_EOK;
}

static rt_int32_t tsc2046_scale(rt_int32_t value,
                                rt_int32_t minimum,
                                rt_int32_t maximum,
                                rt_int32_t size)
{
    if (maximum <= minimum || size <= 0)
    {
        return 0;
    }

    if (value <= minimum)
    {
        return 0;
    }
    if (value >= maximum)
    {
        return size - 1;
    }

    return ((value - minimum) * (size - 1)) / (maximum - minimum);
}

static rt_size_t tsc2046_read_point(struct rt_touch_device *touch,
                                    void *buf,
                                    rt_size_t touch_num)
{
    struct tsc2046_data *ts;
    struct rt_touch_data *data;
    struct tsc2046_sample sample = {0};
    rt_int32_t raw_x;
    rt_int32_t raw_y;
    rt_int32_t x;
    rt_int32_t y;
    rt_int32_t irq_before;
    rt_int32_t irq_after;
    rt_err_t sample_status = -RT_ERROR;

    if (buf == RT_NULL || touch_num == 0)
    {
        return 0;
    }

    ts = rt_container_of(touch, struct tsc2046_data, touch);
    data = (struct rt_touch_data *)buf;
    rt_memset(data, 0, sizeof(*data) * touch_num);
    data->track_id = 0;
    data->width = 1;
    data->timestamp = rt_touch_get_ts();

    if (!ts->initialized)
    {
        rt_pin_write(ts->spi->cs_pin, PIN_HIGH);
        if (tsc2046_power_down(ts) == RT_EOK)
        {
            rt_thread_mdelay(1);
            ts->initialized = RT_TRUE;
        }
        else
        {
            LOG_W("Initial power-down transfer failed");
        }
    }

    irq_before = rt_pin_read(ts->irq_pin);
    irq_after = irq_before;
    if (irq_before == PIN_LOW)
    {
        sample_status = tsc2046_read_state(ts, &sample);
        if (ts->penirq_recheck_delay_usecs)
        {
            rt_hw_us_delay(ts->penirq_recheck_delay_usecs);
        }
        irq_after = rt_pin_read(ts->irq_pin);
    }

    if (ts->initialized &&
        (irq_before != PIN_LOW || irq_after != PIN_LOW))
    {
        data->event = ts->touching ? RT_TOUCH_EVENT_UP : RT_TOUCH_EVENT_NONE;
        data->x_coordinate = ts->last_x;
        data->y_coordinate = ts->last_y;
        ts->touching = RT_FALSE;
        return 1;
    }

    if (sample_status != RT_EOK)
    {
        data->event = RT_TOUCH_EVENT_NONE;
        data->x_coordinate = ts->last_x;
        data->y_coordinate = ts->last_y;
        return 1;
    }

    raw_x = sample.x;
    raw_y = sample.y;
    if (raw_x <= 0 || raw_y <= 0 ||
        raw_x >= TSC2046_MAX_12BIT || raw_y >= TSC2046_MAX_12BIT ||
        sample.pressure == 0 || sample.pressure > ts->pressure_max)
    {
        data->event = RT_TOUCH_EVENT_NONE;
        data->x_coordinate = ts->last_x;
        data->y_coordinate = ts->last_y;
        return 1;
    }

    if (ts->swap_xy)
    {
        x = tsc2046_scale(raw_y, ts->raw_y_min, ts->raw_y_max, ts->size_x);
        y = tsc2046_scale(raw_x, ts->raw_x_min, ts->raw_x_max, ts->size_y);
    }
    else
    {
        x = tsc2046_scale(raw_x, ts->raw_x_min, ts->raw_x_max, ts->size_x);
        y = tsc2046_scale(raw_y, ts->raw_y_min, ts->raw_y_max, ts->size_y);
    }
    if (ts->invert_x)
    {
        x = ts->size_x - 1 - x;
    }
    if (ts->invert_y)
    {
        y = ts->size_y - 1 - y;
    }

    ts->last_x = (rt_uint16_t)x;
    ts->last_y = (rt_uint16_t)y;
    ts->last_pressure = sample.pressure > 0xffff ?
                        0xffff : (rt_uint16_t)sample.pressure;
    LOG_D("diag DOWN raw_x=%d raw_y=%d z1=%d z2=%d pressure=%u x=%u y=%u",
            raw_x, raw_y, sample.z1, sample.z2,
            (unsigned int)sample.pressure, ts->last_x, ts->last_y);
    data->x_coordinate = ts->last_x;
    data->y_coordinate = ts->last_y;
    data->event = ts->touching ? RT_TOUCH_EVENT_MOVE : RT_TOUCH_EVENT_DOWN;
    ts->touching = RT_TRUE;

    return 1;
}

static rt_err_t tsc2046_control(struct rt_touch_device *touch,
                                int command,
                                void *arg)
{
    struct tsc2046_data *ts;

    ts = rt_container_of(touch, struct tsc2046_data, touch);
    if (arg == RT_NULL)
    {
        return -RT_EINVAL;
    }

    switch (command)
    {
    case RT_TOUCH_CTRL_GET_ID:
        rt_memcpy(arg, "2046", 5);
        break;
    case RT_TOUCH_CTRL_GET_INFO:
        rt_memcpy(arg, &touch->info, sizeof(touch->info));
        break;
    case RT_TOUCH_CTRL_SET_X_RANGE:
        if (*(rt_int32_t *)arg <= 0 || *(rt_int32_t *)arg > 65535)
        {
            return -RT_EINVAL;
        }
        ts->size_x = *(rt_int32_t *)arg;
        touch->info.range_x = ts->size_x;
        break;
    case RT_TOUCH_CTRL_SET_Y_RANGE:
        if (*(rt_int32_t *)arg <= 0 || *(rt_int32_t *)arg > 65535)
        {
            return -RT_EINVAL;
        }
        ts->size_y = *(rt_int32_t *)arg;
        touch->info.range_y = ts->size_y;
        break;
    default:
        return -RT_EINVAL;
    }

    return RT_EOK;
}

static const struct rt_touch_ops tsc2046_touch_ops =
{
    .touch_readpoint = tsc2046_read_point,
    .touch_control = tsc2046_control,
};

static rt_uint32_t tsc2046_read_u16_default(struct rt_ofw_node *np,
                                            const char *property,
                                            rt_uint32_t default_value)
{
    rt_uint16_t value;

    if (rt_ofw_prop_read_u16(np, property, &value) == RT_EOK)
    {
        return value;
    }
    return default_value;
}

static void tsc2046_read_config(struct rt_ofw_node *np,
                                struct tsc2046_data *ts)
{
    rt_uint32_t value;

    ts->raw_x_min = tsc2046_read_u16_default(np, "ti,x-min", ts->raw_x_min);
    ts->raw_x_max = tsc2046_read_u16_default(np, "ti,x-max", ts->raw_x_max);
    ts->raw_y_min = tsc2046_read_u16_default(np, "ti,y-min", ts->raw_y_min);
    ts->raw_y_max = tsc2046_read_u16_default(np, "ti,y-max", ts->raw_y_max);

    rt_ofw_prop_read_u32(np, "touchscreen-size-x", (rt_uint32_t *)&ts->size_x);
    rt_ofw_prop_read_u32(np, "touchscreen-size-y", (rt_uint32_t *)&ts->size_y);

    ts->settle_delay_usecs = tsc2046_read_u16_default(
        np, "ti,settle-delay-usec", ts->settle_delay_usecs);
    if (rt_ofw_prop_read_u32(np, "ti,penirq-recheck-delay-usecs",
                             &value) == RT_EOK)
    {
        ts->penirq_recheck_delay_usecs = value;
    }
    ts->x_plate_ohms = tsc2046_read_u16_default(
        np, "ti,x-plate-ohms", ts->x_plate_ohms);
    ts->pressure_max = tsc2046_read_u16_default(
        np, "ti,pressure-max", ts->pressure_max);

    if (rt_ofw_prop_read_u32(np, "touchscreen-max-pressure", &value) == RT_EOK)
    {
        ts->pressure_max = value;
    }
    ts->swap_xy = rt_ofw_prop_read_bool(np, "touchscreen-swapped-x-y") ||
                  rt_ofw_prop_read_bool(np, "ti,swap-xy");
    ts->invert_x = rt_ofw_prop_read_bool(np, "touchscreen-inverted-x");
    ts->invert_y = rt_ofw_prop_read_bool(np, "touchscreen-inverted-y");
    ts->keep_vref_on = rt_ofw_prop_read_bool(np, "ti,keep-vref-on");
}

static rt_int32_t tsc2046_irq_pin(struct rt_ofw_node *np)
{
    struct rt_ofw_cell_args args;
    rt_uint32_t base;
    rt_uint32_t count;
    rt_int32_t pin = -RT_EINVAL;

    if (rt_ofw_parse_phandle_cells(np, "pendown-gpio", "#gpio-cells", 0, &args))
    {
        return -RT_EINVAL;
    }

    if (args.args_count == 2 && args.args[1] == 1 &&
        !rt_ofw_prop_read_u32_index(args.data, "gpio-ranges", 2, &base) &&
        !rt_ofw_prop_read_u32_index(args.data, "gpio-ranges", 3, &count) &&
        args.args[0] < count)
    {
        pin = base + args.args[0];
    }
    rt_ofw_node_put(args.data);
    return pin;
}

static rt_err_t tsc2046_probe(struct rt_spi_device *spi)
{
    struct tsc2046_data *ts;
    struct rt_ofw_node *np;
    struct rt_touch_config touch_config = {0};
    rt_err_t ret;

    np = spi->parent.ofw_node;
    ts = rt_calloc(1, sizeof(*ts));
    if (ts == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    ts->spi = spi;
    ts->raw_x_min = TSC2046_DEFAULT_X_MIN;
    ts->raw_x_max = TSC2046_DEFAULT_X_MAX;
    ts->raw_y_min = TSC2046_DEFAULT_Y_MIN;
    ts->raw_y_max = TSC2046_DEFAULT_Y_MAX;
    ts->size_x = TSC2046_DEFAULT_X_SIZE;
    ts->size_y = TSC2046_DEFAULT_Y_SIZE;
    ts->x_plate_ohms = TSC2046_DEFAULT_X_PLATE_OHMS;
    ts->pressure_max = TSC2046_DEFAULT_PRESSURE_MAX;
    tsc2046_read_config(np, ts);

    if (ts->raw_x_max > TSC2046_MAX_12BIT ||
        ts->raw_y_max > TSC2046_MAX_12BIT ||
        ts->raw_x_min >= ts->raw_x_max || ts->raw_y_min >= ts->raw_y_max ||
        ts->size_x <= 0 || ts->size_x > 65535 ||
        ts->size_y <= 0 || ts->size_y > 65535 ||
        !ts->x_plate_ohms || !ts->pressure_max)
    {
        LOG_E("Invalid calibration or display size");
        ret = -RT_EINVAL;
        goto fail;
    }

    ts->irq_pin = tsc2046_irq_pin(np);
    if (ts->irq_pin < 0)
    {
        LOG_E("Invalid active-low pendown-gpio or GPIO bank range");
        ret = -RT_EINVAL;
        goto fail;
    }

    spi->config.mode = RT_SPI_MODE_0 | RT_SPI_MSB;
    spi->config.data_width = 8;
    if (!spi->config.max_hz || spi->config.max_hz > TSC2046_DEFAULT_MAX_HZ)
    {
        spi->config.max_hz = TSC2046_DEFAULT_MAX_HZ;
    }

    ret = tsc2046_prepare_transfer(ts);
    if (ret != RT_EOK)
    {
        LOG_E("Invalid settle delay or transfer buffer allocation failed");
        goto fail;
    }

    rt_pin_mode(ts->irq_pin, PIN_MODE_INPUT_PULLUP);

    touch_config.irq_pin.pin = ts->irq_pin;
    touch_config.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    touch_config.dev_name = (char *)rt_dm_dev_get_name(&spi->parent);
    touch_config.user_data = ts;

    ts->touch.info.type = RT_TOUCH_TYPE_RESISTANCE;
    ts->touch.info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
    ts->touch.info.point_num = 1;
    ts->touch.info.range_x = ts->size_x;
    ts->touch.info.range_y = ts->size_y;
    ts->touch.config = touch_config;
    ts->touch.ops = &tsc2046_touch_ops;

    ret = rt_hw_touch_register(&ts->touch, "touch", RT_DEVICE_FLAG_INT_RX, ts);
    if (ret != RT_EOK)
    {
        LOG_E("TSC2046 touch register failed: %d", ret);
        goto fail;
    }

    LOG_I("TSC2046 ready irq=%d samples=%u settle=%u/%uus pressure=%u",
          ts->irq_pin, (unsigned int)TSC2046_FILTER_SAMPLES,
          (unsigned int)ts->settle_samples,
          (unsigned int)ts->settle_delay_usecs,
          (unsigned int)ts->pressure_max);
    return RT_EOK;

fail:
    rt_free(ts->tx_buf);
    rt_free(ts->rx_buf);
    rt_free(ts);
    return ret;
}

static const struct rt_spi_device_id tsc2046_ids[] =
{
    { "tsc2046", RT_NULL },
    { "", RT_NULL },
};

static const struct rt_ofw_node_id tsc2046_ofw_ids[] =
{
    { .compatible = "ti,tsc2046" },
    { /* sentinel */ }
};

static struct rt_spi_driver tsc2046_driver =
{
    .ids = tsc2046_ids,
    .ofw_ids = tsc2046_ofw_ids,
    .probe = tsc2046_probe,
};

static int tsc2046_register(void)
{
    return rt_spi_driver_register(&tsc2046_driver);
}
INIT_DEVICE_EXPORT(tsc2046_register);
