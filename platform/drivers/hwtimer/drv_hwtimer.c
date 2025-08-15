/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <drv_hwtimer.h>

// #define DRV_DEBUG
#define DBG_TAG "drv.hwtimer"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_WARNING
#endif /* DRV_DEBUG */
#include <rtdbg.h>

#define TIMER_COUNT 6

enum rk_timer_mode
{
    free_mode = 0,
    user_mode = 1,
};
typedef int rk_timer_mode_t;

struct rk3506_timer_device
{
    void *addr;
    rt_uint32_t irq;
    char name[10];
    struct rt_clk *clk;
    struct rt_clk *pclk;
    rt_uint8_t irq_flag;
    rt_uint8_t init_cnt;
    rt_uint8_t timer_cnt;
    rk_timer_mode_t mode;
    struct rt_hwtimer_device device;
};

static void rk3506_timer_handle(int vector, void *param)
{
    struct rk3506_timer_device *timer = (struct rk3506_timer_device *)param;

    timer->irq_flag = 1;

    if (timer->device.info->cntmode == HWTIMER_CNTMODE_DW)
    {
        timer->timer_cnt =
            timer->init_cnt - readl(timer->addr + TIMER_CURRENT_VALUE0);
    }
    else
    {
        timer->timer_cnt = readl(timer->addr + TIMER_CURRENT_VALUE0);
    }

    rt_device_hwtimer_isr(&timer->device);

    writel(INTSTATUS_EN, timer->addr + TIMER_INTSTATUS);

    timer->irq_flag = 0;
}

static void rk_init(struct rt_hwtimer_device *dev, rt_uint32_t state)
{
    rt_uint8_t reg = 0;
    struct rk3506_timer_device *timer =
        rt_container_of(dev, struct rk3506_timer_device, device);

    if (state == RT_TRUE)
    {
        reg &= ~CONTROL_TIMER_EN;
        reg |= CONTROL_TIMER_MODE;
        reg |= CONTROL_TIMER_INT_EN;

        writel(reg, timer->addr + TIMER_CONTROLREG);
    }
    else if (state == RT_FALSE)
    {
        reg = readl(timer->addr + TIMER_CONTROLREG);

        reg &= ~CONTROL_TIMER_EN;
        reg &= ~CONTROL_TIMER_INT_EN;

        writel(reg, timer->addr + TIMER_CONTROLREG);

        rt_hw_interrupt_mask(timer->irq);
    }
}

static rt_err_t rk_start(
    struct rt_hwtimer_device *dev, rt_uint32_t cnt, rt_hwtimer_mode_t mode)
{
    rt_uint8_t reg = 0;
    int ret = RT_EOK;
    struct rk3506_timer_device *timer =
        rt_container_of(dev, struct rk3506_timer_device, device);

    reg = readl(timer->addr + TIMER_CONTROLREG);

    writel(cnt - 1, timer->addr + TIMER_LOAD_COUNT0);
    writel(0, timer->addr + TIMER_LOAD_COUNT1);

    timer->init_cnt = cnt;

    if (mode == HWTIMER_MODE_ONESHOT)
    {
        timer->mode = user_mode;
        reg |= CONTROL_TIMER_MODE;
    }
    else
    {
        timer->mode = free_mode;
        reg &= ~CONTROL_TIMER_MODE;
    }

    rt_hw_interrupt_umask(timer->irq);

    reg |= 1;
    writel(reg, timer->addr + TIMER_CONTROLREG);

    return ret;
}

static void rk_stop(struct rt_hwtimer_device *dev)
{
    rt_uint32_t reg = 0;
    struct rk3506_timer_device *timer =
        rt_container_of(dev, struct rk3506_timer_device, device);

    reg = readl(timer->addr + TIMER_CONTROLREG);

    /* disable the timer */
    reg &= ~CONTROL_TIMER_EN;

    writel(reg, timer->addr + TIMER_CONTROLREG);

    /* mask interrupt */
    rt_hw_interrupt_mask(timer->irq);
}

static rt_uint32_t rk_count_get(struct rt_hwtimer_device *dev)
{
    struct rk3506_timer_device *timer =
        rt_container_of(dev, struct rk3506_timer_device, device);

    if ((timer->irq_flag) && (timer->mode == user_mode))
    {
        return timer->timer_cnt;
    }

    return readl(timer->addr + TIMER_CURRENT_VALUE0);
}

static rt_err_t rk_control(
    struct rt_hwtimer_device *timer, rt_uint32_t cmd, void *args)
{
    return RT_EOK;
}

static struct rt_hwtimer_ops rk3506_timer_device_ops = { .init = rk_init,
    .start = rk_start,
    .stop = rk_stop,
    .count_get = rk_count_get,
    .control = rk_control };

static struct rt_hwtimer_info rk_hwtimer[2] = {
    /* timer0 timer1 timer2 timer3 timer4 */
    {
        .cntmode = HWTIMER_CNTMODE_DW,
        .maxcnt = RT_UINT32_MAX,
        .maxfreq = 24000000,
        .minfreq = 24000000,
    },
    /* timer5 stimer0 stimer1 */
    {
        .cntmode = HWTIMER_CNTMODE_UP,
        .maxcnt = RT_UINT32_MAX,
        .maxfreq = 24000000,
        .minfreq = 24000000,
    }
};

static struct rk3506_timer_device device0 = { .irq_flag = RT_FALSE,
    .timer_cnt = 0,
    .mode = user_mode,
    .device.info = &rk_hwtimer[0] };

static rt_err_t rk3506_timer_init(struct rt_device *dev)
{
    rt_err_t err = RT_EOK;
    rt_uint32_t reg = 0;
    struct rt_ofw_node *np = dev->ofw_node;
    struct rk3506_timer_device *timer[TIMER_COUNT] = { 0 };
    void *base = rt_ofw_iomap(np, 0);

    for (int i = 0; i < TIMER_COUNT; i++)
    {
        timer[i] = rt_calloc(1, sizeof(struct rk3506_timer_device));

        if (timer[i] == RT_NULL)
        {
            LOG_E("alloc memory for rk3506_timer_device failed");
            goto _fail;
        }

        rt_memcpy(timer[i], &device0, sizeof(struct rk3506_timer_device));

        if (!(timer[i]->clk = rt_clk_find(np, "timer")))
        {
            err = (-RT_EIO);
            goto _fail;
        }

        if (!(timer[i]->pclk = rt_clk_find(np, "pclk")))
        {
            err = (-RT_EIO);
            goto _fail;
        }

        rt_clk_enable(timer[i]->clk);
        rt_clk_enable(timer[i]->pclk);

        timer[i]->addr = base + i * 0x1000;
        if (!timer[i]->addr)
        {
            err = (-RT_EIO);
            goto _fail;
        }

        timer[i]->irq = rt_dm_dev_get_irq(dev, i);
        timer[i]->device.ops = &rk3506_timer_device_ops;

        /* disable the timer */
        reg &= ~(1 << 0);
        /* set timer mode: user-defined mode */
        reg |= 1 << 1;
        writel(reg, timer[i]->addr + TIMER_CONTROLREG);

        rt_sprintf(timer[i]->name, "timer%d", i);

        err = rt_device_hwtimer_register(
            &timer[i]->device, timer[i]->name, timer[i]);
        if (err != RT_EOK)
        {
            LOG_E("%s register fail", timer[i]->name);
            err = (-RT_ERROR);
        }

        rt_hw_interrupt_install(timer[i]->irq, rk3506_timer_handle,
            (void *)timer[i], timer[i]->name);
    }

    return err;

_fail:
    for (int i = 0; i < TIMER_COUNT; i++)
    {
        if (timer[i])
        {
            if (timer[i]->addr)
            {
#if 0
                rt_iounmap(timer[i]->addr);
#endif
            }

            if (timer[i]->pclk)
            {
                rt_clk_disable(timer[i]->pclk);
            }

            if (timer[i]->clk)
            {
                rt_clk_disable(timer[i]->clk);
            }

            rt_free(timer[i]);
        }
    }
    return err;
}

static rt_err_t rk3506_timer_probe(struct rt_platform_device *pdev)
{
    if (rk3506_timer_init(&pdev->parent))
    {
        LOG_E("rk3568 timer init failed");
        return (-RT_ERROR);
    }

    LOG_I("rk3506_timer_probe");

    return RT_EOK;
}

static const struct rt_ofw_node_id rk3506_timer_ofw_ids[] = {
    { .compatible = "rockchip,rk3506-timer" }, { /* sentinel */ }
};

static struct rt_platform_driver rk3506_timer_driver = {
    .name = "rk3506_timer",
    .ids = rk3506_timer_ofw_ids,
    .probe = rk3506_timer_probe,
};

int rk3506_timer_drv_register(void)
{
    return rt_platform_driver_register(&rk3506_timer_driver);
}
INIT_DEVICE_EXPORT(rk3506_timer_drv_register);