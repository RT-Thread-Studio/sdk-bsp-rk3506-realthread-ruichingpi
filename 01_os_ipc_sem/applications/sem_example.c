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

#define THREAD_PRIORITY   6
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

#define MAXSEM            5

rt_uint32_t array[MAXSEM];
static rt_uint32_t set, get;
struct rt_semaphore sem_lock;
struct rt_semaphore sem_empty, sem_full;

void producer_thread_entry(void *parameter)
{
    int cnt = 0;

    while (cnt < 10)
    {
        rt_sem_take(&sem_empty, RT_WAITING_FOREVER);

        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        array[set % MAXSEM] = cnt + 1;
        rt_kprintf(
            "the producer generates a number: %d\n", array[set % MAXSEM]);
        set++;
        rt_sem_release(&sem_lock);

        rt_sem_release(&sem_full);
        cnt++;

        rt_thread_mdelay(20);
    }

    rt_kprintf("the producer exit!\n");
    cnt = 0;
}

void consumer_thread_entry(void *parameter)
{
    rt_uint32_t sum = 0;

    while (1)
    {
        rt_sem_take(&sem_full, RT_WAITING_FOREVER);

        rt_sem_take(&sem_lock, RT_WAITING_FOREVER);
        sum += array[get % MAXSEM];
        rt_kprintf("the consumer[%d] get a number: %d\n", (get % MAXSEM),
            array[get % MAXSEM]);
        get++;
        rt_sem_release(&sem_lock);

        rt_sem_release(&sem_empty);

        if (get == 10)
            break;

        rt_thread_mdelay(50);
    }

    rt_kprintf("the consumer sum is: %d\n", sum);
    rt_kprintf("the consumer exit!\n");
    rt_sem_detach(&sem_lock);
    rt_sem_detach(&sem_empty);
    rt_sem_detach(&sem_full);
    sum = 0;
}

int semaphore_producer_consumer_example(void)
{
    rt_thread_t producer_tid = RT_NULL;
    rt_thread_t consumer_tid = RT_NULL;
    set = 0;
    get = 0;

    rt_sem_init(&sem_lock, "lock", 1, RT_IPC_FLAG_PRIO);
    rt_sem_init(&sem_empty, "empty", MAXSEM, RT_IPC_FLAG_PRIO);
    rt_sem_init(&sem_full, "full", 0, RT_IPC_FLAG_PRIO);

    producer_tid = rt_thread_create("producer", producer_thread_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (producer_tid != RT_NULL)
    {
        rt_thread_startup(producer_tid);
    }
    else
    {
        rt_kprintf("create thread producer failed");
        return -1;
    }

    consumer_tid = rt_thread_create("consumer", consumer_thread_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (consumer_tid != RT_NULL)
    {
        rt_thread_startup(consumer_tid);
    }
    else
    {
        rt_kprintf("create thread consumer failed");
        return -1;
    }

    return 0;
}
MSH_CMD_EXPORT(
    semaphore_producer_consumer_example, semaphore_producer_consumer_example);
