/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __RTHW_H__
#define __RTHW_H__

#include <rtdef.h>
#include <kpi.h>

/*
 * Some macros define
 */
#ifndef HWREG64
#define HWREG64(x)      (*((volatile rt_uint64_t *)(x)))
#endif /* HWREG64 */
#ifndef HWREG32
#define HWREG32(x)      (*((volatile rt_uint32_t *)(x)))
#endif /* HWREG32 */
#ifndef HWREG16
#define HWREG16(x)      (*((volatile rt_uint16_t *)(x)))
#endif /* HWREG16 */
#ifndef HWREG8
#define HWREG8(x)       (*((volatile rt_uint8_t *)(x)))
#endif /* HWREG8 */

enum RT_HW_CACHE_OPS
{
    RT_HW_CACHE_FLUSH      = 0x01,
    RT_HW_CACHE_INVALIDATE = 0x02,
};

/*
 * Cache interfaces
 */
typedef void (*__kpi_rt_hw_cpu_icache_enable)(void);
typedef void (*__kpi_rt_hw_cpu_icache_disable)(void);
typedef rt_base_t (*__kpi_rt_hw_cpu_icache_status)(void);
typedef void (*__kpi_rt_hw_cpu_icache_ops)(int ops, void* addr, int size);
typedef void (*__kpi_rt_hw_cpu_dcache_enable)(void);
typedef void (*__kpi_rt_hw_cpu_dcache_disable)(void);
typedef rt_base_t (*__kpi_rt_hw_cpu_dcache_status)(void);
typedef void (*__kpi_rt_hw_cpu_dcache_ops)(int ops, void* addr, int size);
typedef void (*__kpi_rt_hw_isb)(void);
typedef void (*__kpi_rt_hw_dmb)(void);
typedef void (*__kpi_rt_hw_dsb)(void);
typedef void (*__kpi_rt_hw_cpu_relax)(void);
typedef void (*__kpi_rt_hw_rmb)(void);

/*
 * Cpu interfaces
 */
typedef void (*__kpi_rt_hw_cpu_reset)(void);
typedef void (*__kpi_rt_hw_cpu_shutdown)(void);
typedef const char *(*__kpi_rt_hw_cpu_arch)(void);
typedef int (*__kpi_rt_hw_cpu_id)(void);


/**
 * boot secondary cpu interfaces
 */
#if 0
void rt_hw_secondary_cpu_up(void);
void rt_hw_secondary_cpu_idle_exec(void);

/* stack interfaces */
rt_uint8_t *rt_hw_stack_init(void *entry, void *parameter,
    rt_uint8_t *stack_addr, void *exit);
void rt_hw_stack_guard_init(rt_thread_t thread);
#endif

/*
 * Interrupt interfaces
 */
typedef void (*__kpi_rt_hw_interrupt_init)(void);
typedef void (*__kpi_rt_hw_interrupt_mask)(int vector);
typedef void (*__kpi_rt_hw_interrupt_umask)(int vector);
typedef rt_isr_handler_t (*__kpi_rt_hw_interrupt_install)(int vector,
    rt_isr_handler_t handler, void *param, const char *name);
typedef void (*__kpi_rt_hw_interrupt_uninstall)(int vector,
    rt_isr_handler_t handler, void *param);
typedef rt_base_t (*__kpi_rt_hw_local_irq_disable)(void);

typedef void (*__kpi_rt_hw_local_irq_enable)(rt_base_t level);
typedef rt_base_t (*__kpi_rt_hw_interrupt_disable)(void);
typedef void (*__kpi_rt_hw_interrupt_enable)(rt_base_t level);
typedef rt_bool_t (*__kpi_rt_hw_interrupt_is_disabled)(void);
typedef void (*__kpi_rt_hw_ipi_send)(int ipi_vector, unsigned int cpu_mask);
typedef void (*__kpi_rt_hw_ipi_handler_install)(int ipi_vector,
    rt_isr_handler_t ipi_isr_handler);

/*
 * Exception interfaces
 */
typedef void (*__kpi_rt_hw_exception_install)(
    rt_err_t (*exception_handle)(void *context));

/*
 * Context interfaces
 */
#if 0
#ifdef RT_USING_SMP
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread);
void rt_hw_context_switch_to(rt_ubase_t to, struct rt_thread *to_thread);
void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread);
#else
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
void rt_hw_context_switch_to(rt_ubase_t to);
void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread, rt_thread_t to_thread);
#endif /* RT_USING_SMP */
#endif

/**
 * Hardware Layer Backtrace Service
 */
#if 0
rt_err_t rt_hw_backtrace_frame_get(rt_thread_t thread,
    struct rt_hw_backtrace_frame *frame);
rt_err_t rt_hw_backtrace_frame_unwind(rt_thread_t thread,
    struct rt_hw_backtrace_frame *frame);

void rt_hw_console_output(const char *str);
#endif

/*
 * delay interfaces
 */
typedef void (*__kpi_rt_hw_us_delay)(rt_uint32_t us);

/*
 * timer interfaces
 */
typedef rt_uint32_t (*__kpi_rt_hw_global_timer_get)(void);

/*
 * spinlock interfaces
 */
extern rt_hw_spinlock_t _cpus_lock;
#define RT_DEFINE_HW_SPINLOCK(x)  rt_hw_spinlock_t x = {0}

#if 0
void rt_hw_spin_lock_init(rt_hw_spinlock_t *lock);
void rt_hw_spin_lock(rt_hw_spinlock_t *lock);
void rt_hw_spin_unlock(rt_hw_spinlock_t *lock);
#endif

/*
 * atomic interfaces
 */
#if 0
rt_atomic_t rt_hw_atomic_load(volatile rt_atomic_t *ptr);
void rt_hw_atomic_store(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_add(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_sub(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_and(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_or(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_xor(volatile rt_atomic_t *ptr, rt_atomic_t val);
rt_atomic_t rt_hw_atomic_exchange(volatile rt_atomic_t *ptr, rt_atomic_t val);
void rt_hw_atomic_flag_clear(volatile rt_atomic_t *ptr);
rt_atomic_t rt_hw_atomic_flag_test_and_set(volatile rt_atomic_t *ptr);
rt_atomic_t rt_hw_atomic_compare_exchange_strong(volatile rt_atomic_t *ptr, 
    rt_atomic_t *expected, rt_atomic_t desired);
#endif

KPI_EXTERN(rt_hw_cpu_icache_enable);
KPI_EXTERN(rt_hw_cpu_icache_disable);
KPI_EXTERN(rt_hw_cpu_icache_ops);
KPI_EXTERN(rt_hw_cpu_icache_status);
KPI_EXTERN(rt_hw_cpu_dcache_enable);
KPI_EXTERN(rt_hw_cpu_dcache_disable);
KPI_EXTERN(rt_hw_cpu_dcache_status);
KPI_EXTERN(rt_hw_cpu_dcache_ops);
KPI_EXTERN(rt_hw_isb);
KPI_EXTERN(rt_hw_dmb);
KPI_EXTERN(rt_hw_dsb);
KPI_EXTERN(rt_hw_cpu_relax);
KPI_EXTERN(rt_hw_rmb);

KPI_EXTERN(rt_hw_cpu_reset);
KPI_EXTERN(rt_hw_cpu_shutdown);
KPI_EXTERN(rt_hw_cpu_arch);
KPI_EXTERN(rt_hw_cpu_id);

KPI_EXTERN(rt_hw_local_irq_disable);
KPI_EXTERN(rt_hw_local_irq_enable);
KPI_EXTERN(rt_hw_interrupt_disable);
KPI_EXTERN(rt_hw_interrupt_enable);
KPI_EXTERN(rt_hw_interrupt_is_disabled);

KPI_EXTERN(rt_hw_ipi_send);
KPI_EXTERN(rt_hw_ipi_handler_install);
KPI_EXTERN(rt_hw_interrupt_mask);
KPI_EXTERN(rt_hw_interrupt_umask);
KPI_EXTERN(rt_hw_interrupt_install);
KPI_EXTERN(rt_hw_interrupt_uninstall);
KPI_EXTERN(rt_hw_exception_install);

KPI_EXTERN(rt_hw_us_delay);

KPI_EXTERN(rt_hw_global_timer_get);

#endif /* __RTHW_H__ */
