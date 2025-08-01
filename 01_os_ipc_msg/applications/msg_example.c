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

#define THREAD1_PRIORITY   25
#define THREAD1_STACK_SIZE 4096
#define THREAD1_TIMESLICE  5

#define THREAD2_PRIORITY   25
#define THREAD2_STACK_SIZE 4096
#define THREAD2_TIMESLICE  5

static struct rt_messagequeue mq;
static rt_uint8_t msg_pool[2048];

static void thread1_entry(void *parameter)
{
    char buf = 0;
    rt_uint8_t cnt = 0;

    while (1)
    {
        if (rt_mq_recv(&mq, &buf, sizeof(buf), RT_WAITING_FOREVER) > 0)
        {
            rt_kprintf(
                "thread1: recv msg from msg queue, the content:%c\n", buf);
            if (cnt == 19)
            {
                break;
            }
        }

        cnt++;
        rt_thread_mdelay(50);
    }

    rt_kprintf("thread1: detach mq \n");
    rt_mq_detach(&mq);
}

static void thread2_entry(void *parameter)
{
    int result;
    char buf = 'A';
    rt_uint8_t cnt = 0;

    while (1)
    {
        if (cnt == 8)
        {
            result = rt_mq_urgent(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_urgent ERR\n");
            }
            else
            {
                rt_kprintf("thread2: send urgent message - %c\n", buf);
            }
        }
        else if (cnt >= 20)
        {
            rt_kprintf("message queue stop send, thread2 quit\n");
            break;
        }
        else
        {
            result = rt_mq_send(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_send ERR\n");
            }

            rt_kprintf("thread2: send message - %c\n", buf);
        }

        buf++;
        cnt++;
        rt_thread_mdelay(5);
    }
}

int msgq_example(void)
{
    rt_err_t result;
    rt_thread_t tid1 = RT_NULL;
    rt_thread_t tid2 = RT_NULL;

    result = rt_mq_init(
        &mq, "mqt", &msg_pool[0], 1, sizeof(msg_pool), RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        rt_kprintf("init message queue failed.\n");
        return -1;
    }

    tid1 = rt_thread_create("thread1", thread1_entry, RT_NULL,
        THREAD1_STACK_SIZE, THREAD1_PRIORITY, THREAD1_TIMESLICE);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }

    tid2 = rt_thread_create("thread2", thread2_entry, RT_NULL,
        THREAD2_STACK_SIZE, THREAD2_PRIORITY, THREAD2_TIMESLICE);
    if (tid2 != RT_NULL)
    {
        rt_thread_startup(tid2);
    }

    return 0;
}
MSH_CMD_EXPORT(msgq_example, msgq_example);
