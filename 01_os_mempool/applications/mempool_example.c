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

#define THREAD_PRIORITY   20
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

#define BLOCK_SIZE        64
#define BLOCK_COUNT       10
#define POOL_SIZE         ((BLOCK_SIZE + sizeof(rt_uint8_t *)) * BLOCK_COUNT)

static rt_uint8_t mempool_pool[POOL_SIZE];
static struct rt_mempool mp;

static rt_uint8_t *block_queue[BLOCK_COUNT];

void mempool_alloc_thread_entry(void *parameter)
{
    int i;

    for (i = 0; i < BLOCK_COUNT; i++)
    {
        block_queue[i] = rt_mp_alloc(&mp, RT_WAITING_FOREVER);
        if (block_queue[i] != RT_NULL)
        {
            rt_kprintf("alloc thread: alloc block[%d] = %p\n", i, block_queue[i]);
        }
    }

    rt_kprintf("alloc thread exit\n");
}

void mempool_free_thread_entry(void *parameter)
{
    int i;

    for (i = 0; i < BLOCK_COUNT; i++)
    {
        rt_mp_free(block_queue[i]);
        rt_kprintf("free thread: free block[%d] = %p\n", i, block_queue[i]);
    }

    rt_kprintf("free thread exit\n");
    rt_mp_detach(&mp);
}

int mempool_example(void)
{
    rt_thread_t alloc_tid = RT_NULL;
    rt_thread_t free_tid = RT_NULL;
    rt_err_t result;

    result = rt_mp_init(&mp, "mempool", mempool_pool, POOL_SIZE, BLOCK_SIZE);
    if (result != RT_EOK)
    {
        rt_kprintf("mp_init failed\n");
        return result;
    }
    
    alloc_tid = rt_thread_create("mp_alloc", mempool_alloc_thread_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (alloc_tid != RT_NULL)
    {
        rt_thread_startup(alloc_tid);
    }
    else
    {
        rt_kprintf("create thread mp_alloc failed\n");
        return -RT_ERROR;
    }

    free_tid = rt_thread_create("mp_free", mempool_free_thread_entry, RT_NULL,
        THREAD_STACK_SIZE, THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (free_tid != RT_NULL)
    {
        rt_thread_startup(free_tid);
    }
    else
    {
        rt_kprintf("create thread mp_free failed\n");
        return -RT_ERROR;
    }

    return 0;
}
MSH_CMD_EXPORT(mempool_example, mempool example);
