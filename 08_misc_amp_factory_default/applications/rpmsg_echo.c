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

struct rt_rpmsg_ep_addr
{
    const char *name;
    rt_uint32_t src;
    rt_uint32_t dst;
};

#define THREAD_PRIORITY         18
#define THREAD_STACK_SIZE       5120
#define THREAD_TIMESLICE        5
#define BUFF_SIZE               256

static struct rt_device *rpmsg = RT_NULL;

struct rt_rpmsg_ep_addr rpmsg_remote_echo = {
    "rpmsg_chrdev", 0x3000U, 0x1000
};

static void rpmsg_echo_thread(void *parameter)
{
    rt_uint32_t len = 0;
    rt_uint8_t buff[BUFF_SIZE];
    while ((len = rt_device_read(rpmsg, rpmsg_remote_echo.src, buff, BUFF_SIZE - 1)) >= 0)
    {
        buff[len] = 0;
        rt_kprintf("message:%s len:%d\r\n", buff, len);
        rt_device_write(rpmsg, rpmsg_remote_echo.dst, buff, len);
    }
}

int rpmsg_echo()
{
    uint32_t ret = RT_EOK;
    rpmsg = rt_device_find("rpmsg");
    if (rpmsg == RT_NULL)
    {
        rt_kprintf("Unable to find rpmsg device.\r\n");
        return RT_ERROR;
    }

    rt_device_open(rpmsg, RT_DEVICE_OFLAG_OPEN);
    rt_device_control(rpmsg, RT_DEVICE_CTRL_CONFIG, &rpmsg_remote_echo);

    static rt_thread_t thread = RT_NULL;
    thread = rt_thread_create("rpmsg_echo",
                            rpmsg_echo_thread, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);

    if(ret != RT_EOK)
    {
        return RT_ERROR;
    }

    rt_kprintf("rpmsg echo init end.\r\n");
    return RT_EOK;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(rpmsg_echo, startup rpmsg_echo server);
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(rpmsg_echo, startup rpmsg_echo server)
#endif /* FINSH_USING_MSH */
#endif /* RT_USING_FINSH */
