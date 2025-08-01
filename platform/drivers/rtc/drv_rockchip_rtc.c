/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "drv_rockchip_rtc.h"

// #define DRV_DEBUG
#define DBG_TAG "drv.rtc"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_WARNING
#endif /* DRV_DEBUG */
#include <rtdbg.h>

struct rk_rtc
{
    struct rt_device parent;
    struct rt_i2c_bus_device *i2c;
    rt_uint32_t addr;
};
typedef struct rk_rtc *rk_rtc_t;

static rt_err_t rk_rtc_init(rt_device_t dev)
{
    rk_rtc_t rtc = (rk_rtc_t)dev;
    rt_uint8_t buf[2] = { 0 };
    struct rt_i2c_msg msg;

    buf[0] = 0x01;
    buf[1] = 0x0A;

    msg.addr = rtc->addr;
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = 2;

    if (rt_i2c_transfer(rtc->i2c, &msg, 1) == 1)
    {
        return RT_EOK;
    }

    return (-RT_ERROR);
}

static rt_err_t rk_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_ssize_t rk_rtc_read(
    rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_err_t rk_rtc_ioctl(rt_device_t dev, int cmd, void *args)
{
    rk_rtc_t rtc = (rk_rtc_t)dev;
    time_t *r_time = RT_NULL;

    if (args == RT_NULL)
    {
        LOG_E("args is NULL");
        return (-RT_ERROR);
    }

    r_time = (time_t *)args;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        hym8563s_get_time(rtc->i2c, rtc->addr, r_time);
        break;
    case RT_DEVICE_CTRL_RTC_SET_TIME:
        hym8563s_set_time(rtc->i2c, rtc->addr, r_time);
        break;
    case RT_DEVICE_CTRL_RTC_GET_ALARM:
    case RT_DEVICE_CTRL_RTC_SET_ALARM: break;
    case RT_DEVICE_CTRL_RTC_GET_TIMEVAL: return (-RT_ENOSYS);
    default: LOG_W("unsupported RTC cmd: %d", cmd); return (-RT_EINVAL);
    }

    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops rtc_device_ops =
{
    .init = rk_rtc_init,
    .open = rk_rtc_open,
    .read = rk_rtc_read,
    .control = rk_rtc_ioctl,
};
#endif

static rt_err_t rtc_hym8563_probe(struct rt_platform_device *pdev)
{
    rt_err_t ret;
    char bus_name[RT_NAME_MAX];
    rt_int32_t id;

    rk_rtc_t rtc = rt_calloc(1, sizeof(struct rk_rtc));
    if (!rtc)
    {
        LOG_E("no mem for rtc");
        return -RT_ENOMEM;
    }

    struct rt_ofw_node *np = pdev->parent.ofw_node;
    struct rt_ofw_node *parent = rt_ofw_get_parent(np);

    ret = rt_ofw_prop_read_u32(np, "reg", &rtc->addr);
    if (ret != RT_EOK)
    {
        LOG_E("rtc i2c addr not found");
        goto __fail;
    }

    id = rt_ofw_get_alias_id(parent, "i2c");
    rt_snprintf(bus_name, RT_NAME_MAX, "i2c%d", id);

    rtc->i2c = rt_i2c_bus_device_find(bus_name);
    if (!rtc->i2c)
    {
        LOG_E("i2c device %s not found", bus_name);
        goto __fail;
    }

    struct rt_device *rtdev = &rtc->parent;
    rtdev->type = RT_Device_Class_RTC;

#ifdef RT_USING_DEVICE_OPS
    rtdev->ops = &rtc_device_ops;
#else
    rtdev->init = rk_rtc_init;
    rtdev->open = rk_rtc_open;
    rtdev->read = rk_rtc_read;
    rtdev->control = rk_rtc_ioctl;
#endif

    ret = rt_device_register(rtdev, "rtc", RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("rtc register failed: %d", ret);
        goto __fail;
    }

    return RT_EOK;

__fail:
    rt_free(rtc);
    return (-RT_ERROR);
}

static const struct rt_ofw_node_id rtc_hym8563_ofw_ids[] =
{
    { .compatible = "haoyu,hym8563" },
    { /* sentinel */ }
};

static struct rt_platform_driver rtc_hym8563_driver =
{
    .name = "hym8563",
    .ids = rtc_hym8563_ofw_ids,
    .probe = rtc_hym8563_probe,
};

static int rtc_hym8563_register(void)
{
    return rt_platform_driver_register(&rtc_hym8563_driver);
}
INIT_APP_EXPORT(rtc_hym8563_register);
