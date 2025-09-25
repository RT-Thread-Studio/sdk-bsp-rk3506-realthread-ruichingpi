/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */
#include <lvgl.h>
#include <stdbool.h>
#include <rtdevice.h>
#define DBG_TAG             "LVGL.port.indev"
#define DBG_LVL             DBG_INFO
#include <rtdbg.h>

#define TOUCH_DEV_NAME      "touch"

static rt_device_t touch_dev;
struct rt_touch_data read_data[5];
static lv_indev_state_t last_state = LV_INDEV_STATE_REL;
static rt_int16_t last_x = 0;
static rt_int16_t last_y = 0;

static void lv_input_thread_entry(void *parameter)
{
    while (1)
    {
        if (RT_NULL == touch_dev)
        {
            break;
        }

        rt_device_read(touch_dev, 0, read_data, 5);

        rt_device_control(touch_dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);

        if (read_data[0].event == RT_TOUCH_EVENT_NONE)
        {
            rt_thread_mdelay(1);
            continue;
        }

        last_x = read_data[0].x_coordinate;
        last_y = read_data[0].y_coordinate;
        if (read_data[0].event == RT_TOUCH_EVENT_DOWN)
        {
            last_state = LV_INDEV_STATE_PRESSED;
        }
        else if (read_data[0].event == RT_TOUCH_EVENT_MOVE)
        {
            last_state = LV_INDEV_STATE_PRESSED;
        }
        else if (read_data[0].event == RT_TOUCH_EVENT_UP)
        {
            last_state = LV_INDEV_STATE_RELEASED;
        }

        rt_thread_mdelay(1);
    }
}

static void input_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = last_state;
}

void lv_port_indev_input(rt_int16_t x, rt_int16_t y, lv_indev_state_t state)
{
    last_state = state;
    last_x = x;
    last_y = y;
}

int lv_port_indev_init(void)
{
    rt_uint8_t id[9];
    struct rt_touch_info info;
    int ret;

    touch_dev = rt_device_find(TOUCH_DEV_NAME);
    if (touch_dev == RT_NULL)
    {
        LOG_E("can't find device:%s", TOUCH_DEV_NAME);
        return (-RT_ENOENT);
    }

    ret = rt_device_open(touch_dev, RT_DEVICE_FLAG_INT_RX);
    if (ret != RT_EOK)
    {
        LOG_E("open device failed!");
        return ret;
    }

    rt_device_control(touch_dev, RT_TOUCH_CTRL_GET_ID, id);
    LOG_I("id = GT%d%d%d", id[0] - '0', id[1] - '0', id[2] - '0');

    rt_device_control(touch_dev, RT_TOUCH_CTRL_GET_INFO, &info);
    LOG_I("range_x = %d", info.range_x);
    LOG_I("range_y = %d", info.range_y);
    LOG_I("point_num = %d", info.point_num);

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, input_read);

    rt_thread_t tid = rt_thread_create("lv_input", lv_input_thread_entry,
        RT_NULL, 10240, 20, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return RT_EOK;
}
