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
#include <string.h>

#define CAN_TEST_NUM 10

#define CAN1_NAME    "ff320000.can"

#define CAN_HEX(label, line, data, length)                                     \
    do                                                                         \
    {                                                                          \
        rt_kprintf("[%s] Hex Dump (%d bytes):\n", label, length);              \
        for (size_t i = 0; i < length; i++)                                    \
        {                                                                      \
            rt_kprintf("%02X ", (unsigned char)data[i]);                       \
            if ((i + 1) % line == 0)                                           \
                rt_kprintf("\n");                                              \
        }                                                                      \
        rt_kprintf("\n");                                                      \
    } while (0)

static rt_device_t dev = RT_NULL;
static rt_uint32_t baud_rate;

static rt_err_t can_send_func(rt_device_t dev, struct rt_can_msg *msg)
{
    rt_err_t status = RT_EOK;

    status = rt_device_write(dev, 0, msg, 1);
    if (status < 0)
    {
        rt_kprintf("send err(0x%d) id = 0x%X fd = %d sync = %d\n", status,
            msg->id, msg->fdf, msg->sync);
        CAN_HEX("err", 16, msg->data, msg->len);
    }
    else if (msg->sync == CAN_SYNC)
    {
        rt_kprintf("send ok id = 0x%X fd = %d sync = %d\n", msg->id, msg->fdf,
            msg->sync);
        CAN_HEX("success", 16, msg->data, msg->len);
    }

    return RT_EOK;
}

static void can_send(rt_device_t dev, rt_uint32_t ver)
{
    struct rt_can_msg msg;
    rt_uint32_t i = 0;
    rt_uint32_t j = 0;

    msg.id = 0x7FF;
    msg.ide = RT_CAN_STDID;
    msg.rtr = RT_CAN_DTR;
    msg.priv = LOW_PRIORITY;
    msg.sync = CAN_SYNC;
    msg.fdf = ver;
    msg.len = 8;
    for (i = 0; i < msg.len; i++)
    {
        msg.data[i] = i;
    }

    for (j = 0; j < CAN_TEST_NUM; j++)
    {
        can_send_func(dev, &msg);

        for (i = 0; i < msg.len; i++)
        {
            msg.data[i] = i + j;
        }
        rt_thread_mdelay(1000);
    }
}

static void can_demo_device_init(rt_device_t dev)
{
    rt_err_t ret = RT_NULL;

    /* Set event callback mask */
    ret = rt_device_control(dev, RT_CAN_CTL_SET_EVENT_CALLBACK_MASK,
        (void *)(RT_CAN_EVENT_SEND_SUCCESS | RT_CAN_EVENT_SEND_FAILURE |
            RT_CAN_EVENT_SEND_TIMEOUT));
    if (ret != RT_EOK)
    {
        rt_kprintf("set event callback mask faild\n");
    }

    ret = rt_device_control(dev, RT_CAN_CTL_SET_BAUD, (void *)baud_rate);
    if (ret != RT_EOK)
    {
        rt_kprintf("set baud faild\n");
    }

    ret = rt_device_open(dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    if (ret != RT_EOK)
    {
        rt_kprintf("can0 open faild\n");
        return;
    }

    can_send(dev, CAN_20);
}

int can_send_test(int argc, char *argv[])
{
    char *name = RT_NULL;

    baud_rate = CAN500kBaud;

    if (argc == 2)
    {
        name = (char *)argv[1];
    }
    else
    {
        rt_kprintf("%s <device name> \n", __func__);
        return -1;
    }

    dev = rt_device_find(name);
    if (dev == RT_NULL)
    {
        rt_kprintf("not found device (%s).\n", name);
        return (-RT_ERROR);
    }

    can_demo_device_init(dev);

    return RT_EOK;
}
MSH_CMD_EXPORT(can_send_test, can send test);
