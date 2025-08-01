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
#include <rtdef.h>

#define THREAD_PRIORITY  9
#define THREAD_TIMESLICE 5

#define EVENT_FLAG3      (1 << 3)
#define EVENT_FLAG5      (1 << 5)

static struct rt_event event;

rt_align(RT_ALIGN_SIZE) static char ev_thread1_stack[4096];
static struct rt_thread ev_thread1;

rt_align(RT_ALIGN_SIZE) static char ev_thread2_stack[4096];
static struct rt_thread ev_thread2;

static void thread1_recv_event(void *param)
{
    rt_uint32_t e;

    if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER,
            &e) == RT_EOK)
    {
        rt_kprintf("thread1: OR recv event 0x%x\n", e);
    }

    rt_kprintf("thread1: delay 1s to prepare the second event\n");
    rt_thread_mdelay(1000);

    if (rt_event_recv(&event, (EVENT_FLAG3 | EVENT_FLAG5),
            RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER,
            &e) == RT_EOK)
    {
        rt_kprintf("thread1: AND recv event 0x%x\n", e);
    }

    rt_event_detach(&event);
    rt_kprintf("thread1 leave.\n");
}

static void thread2_send_event(void *param)
{
    rt_kprintf("thread2: send event3\n");
    rt_event_send(&event, EVENT_FLAG3);
    rt_thread_mdelay(200);

    rt_kprintf("thread2: send event5\n");
    rt_event_send(&event, EVENT_FLAG5);
    rt_thread_mdelay(200);

    rt_kprintf("thread2: send event3\n");
    rt_event_send(&event, EVENT_FLAG3);
    rt_kprintf("thread2 leave.\n");
}

int event_example(void)
{
    rt_err_t result;

    result = rt_event_init(&event, "event", RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event failed.\n");
        return -1;
    }

    rt_thread_init(&ev_thread1, "thread1", thread1_recv_event, RT_NULL,
        &ev_thread1_stack[0], sizeof(ev_thread1_stack), THREAD_PRIORITY - 1,
        THREAD_TIMESLICE);
    rt_thread_startup(&ev_thread1);

    rt_thread_init(&ev_thread2, "thread2", thread2_send_event, RT_NULL,
        &ev_thread2_stack[0], sizeof(ev_thread2_stack), THREAD_PRIORITY,
        THREAD_TIMESLICE);
    rt_thread_startup(&ev_thread2);

    return 0;
}
MSH_CMD_EXPORT(event_example, event_example);
