/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RTATOMIC_H__
#define __RTATOMIC_H__

#include <rthw.h>
#include <kpi.h>

typedef rt_atomic_t (*__kpi_rt_atomic_load)(volatile rt_atomic_t *ptr);
typedef void (*__kpi_rt_atomic_store)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_add)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_sub)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_and)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_or)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_xor)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef rt_atomic_t (*__kpi_rt_atomic_exchange)(volatile rt_atomic_t *ptr,
    rt_atomic_t val);
typedef void (*__kpi_rt_atomic_flag_clear)(volatile rt_atomic_t *ptr);
typedef rt_atomic_t (*__kpi_rt_atomic_flag_test_and_set)(volatile
    rt_atomic_t *ptr);
typedef rt_atomic_t (*__kpi_rt_atomic_compare_exchange_strong)(volatile
    rt_atomic_t *ptr,
    rt_atomic_t *expected, rt_atomic_t desired);

KPI_EXTERN(rt_atomic_load);
KPI_EXTERN(rt_atomic_store);
KPI_EXTERN(rt_atomic_add);
KPI_EXTERN(rt_atomic_sub);
KPI_EXTERN(rt_atomic_and);
KPI_EXTERN(rt_atomic_or);
KPI_EXTERN(rt_atomic_xor);
KPI_EXTERN(rt_atomic_exchange);
KPI_EXTERN(rt_atomic_flag_clear);
KPI_EXTERN(rt_atomic_flag_test_and_set);
KPI_EXTERN(rt_atomic_compare_exchange_strong);

rt_inline rt_atomic_t rt_soft_atomic_exchange(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr = val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_add(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr += val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_sub(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr -= val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_xor(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr = (*ptr) ^ val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_and(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr = (*ptr) & val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_or(volatile rt_atomic_t *ptr,
    rt_atomic_t val)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    *ptr = (*ptr) | val;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_atomic_t rt_soft_atomic_load(volatile rt_atomic_t *ptr)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    temp = *ptr;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline void rt_soft_atomic_store(volatile rt_atomic_t *ptr, rt_atomic_t val)
{
    rt_base_t level;
    level = rt_hw_interrupt_disable();
    *ptr = val;
    rt_hw_interrupt_enable(level);
}

rt_inline rt_atomic_t rt_soft_atomic_flag_test_and_set(volatile rt_atomic_t *ptr)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    if (*ptr == 0)
    {
        temp = 0;
        *ptr = 1;
    }
    else
        temp = 1;
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline void rt_soft_atomic_flag_clear(volatile rt_atomic_t *ptr)
{
    rt_base_t level;
    level = rt_hw_interrupt_disable();
    *ptr = 0;
    rt_hw_interrupt_enable(level);
}

rt_inline rt_atomic_t rt_soft_atomic_compare_exchange_strong(
    volatile rt_atomic_t *ptr1, rt_atomic_t *ptr2,
        rt_atomic_t desired)
{
    rt_base_t level;
    rt_atomic_t temp;
    level = rt_hw_interrupt_disable();
    if ((*ptr1) != (*ptr2))
    {
        *ptr2 = *ptr1;
        temp = 0;
    }
    else
    {
        *ptr1 = desired;
        temp = 1;
    }
    rt_hw_interrupt_enable(level);
    return temp;
}

rt_inline rt_bool_t rt_atomic_dec_and_test(volatile rt_atomic_t *ptr)
{
    return rt_atomic_sub(ptr, 1) == 1;
}

rt_inline rt_atomic_t rt_atomic_fetch_add_unless(volatile rt_atomic_t *ptr,
    rt_atomic_t a, rt_atomic_t u)
{
    rt_atomic_t c = rt_atomic_load(ptr);

    do {
        if (c == u)
        {
            break;
        }
    } while (!rt_atomic_compare_exchange_strong(ptr, &c, c + a));

    return c;
}

rt_inline rt_bool_t rt_atomic_add_unless(volatile rt_atomic_t *ptr,
    rt_atomic_t a, rt_atomic_t u)
{
    return rt_atomic_fetch_add_unless(ptr, a, u) != u;
}

rt_inline rt_bool_t rt_atomic_inc_not_zero(volatile rt_atomic_t *ptr)
{
    return rt_atomic_add_unless(ptr, 1, 0);
}

/**
 * @brief initialize a lock-less single list
 *
 * @param l the single list to be initialized
 */
rt_inline void rt_ll_slist_init(rt_ll_slist_t *l)
{
    l->next = 0;
}

rt_inline void rt_ll_slist_enqueue(rt_ll_slist_t *l, rt_ll_slist_t *n)
{
    rt_base_t exp;
    exp = rt_atomic_load(&l->next);
    do
    {
        n->next = exp;
    } while (!rt_atomic_compare_exchange_strong(&l->next, &exp, (rt_base_t)n));
}

rt_inline rt_ll_slist_t *rt_ll_slist_dequeue(rt_ll_slist_t *l)
{
    rt_base_t exp;
    rt_ll_slist_t *head;

    exp = rt_atomic_load(&l->next);
    do
    {
        head = (rt_ll_slist_t *)exp;
    } while (head && !rt_atomic_compare_exchange_strong(&l->next, &exp,
        rt_atomic_load(&head->next)));
    return head;
}

#endif /* __RTATOMIC_H__ */
