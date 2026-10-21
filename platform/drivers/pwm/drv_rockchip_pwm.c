/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <rtdevice.h>
#include <drv_rockchip_pwm.h>

#define PWM_CMD_SET_CAPTURE_CALLBACK (RT_DEVICE_CTRL_BASE(PWM) + 13)

struct pwm_capture_callback
{
    void (*callback)(void *param, rt_int32_t status,
                     rt_uint32_t hpc, rt_uint32_t lpc,
                     rt_uint32_t freq);
    void *param;
};

struct pwm_device
{
    rt_uint32_t channel;
    rt_uint32_t freq;
    void *base;
    rt_uint32_t phase;
    rt_int32_t irq;
    rt_uint32_t capture_on;
    rt_uint32_t biphasic_on;
    void (*biphasic_callback)(void *param, rt_int32_t count);
    void *biphasic_param;
    void (*capture_callback)(void *param, rt_int32_t status,
                             rt_uint32_t hpc, rt_uint32_t lpc,
                             rt_uint32_t freq);
    void *capture_param;
    struct rt_clk *clk;
    struct rt_device_pwm pwm;
};

static void __pwm_disable(struct pwm_device *pwm_dev);

static rt_err_t __pwm_biphasic_config(struct pwm_device *pwm_dev,
                                      const struct rt_pwm_biphasic_config *cfg)
{
    rt_uint32_t ctrl0, ctrl1;
    rt_uint32_t base = (rt_uint32_t)pwm_dev->base;

    if (!cfg || cfg->mode > 4 || cfg->filter_number > 127)
        return -RT_EINVAL;

    /* RK3506 TRM: biphasic counter exists only in PWM1, channels 0..6.
     * Channel 6 owns the counter registers; channel 7 is only its B input. */
    if ((base & 0xffff0000U) != 0xff170000U || pwm_dev->channel > 6)
        return -RT_ENOSYS;

    __pwm_disable(pwm_dev);

    /* Keep the synchronized result path enabled so the count can be read
     * without waiting for the timer interrupt. */
    ctrl0 = PWM_BIPHASIC_CTRL0_MODE(cfg->mode) |
            PWM_BIPHASIC_CTRL0_SYNC_EN |
            (cfg->continuous ? PWM_BIPHASIC_CTRL0_CONTINUOUS : 0);
    /* Write-enable mirrors every writable low-half bit (TRM CTRL0[31:16]).
     * This includes mode bit5, which is required for mode 4. */
    ctrl0 |= PWM_BIPHASIC_CTRL0_WE(0) | PWM_BIPHASIC_CTRL0_WE(1) |
             PWM_BIPHASIC_CTRL0_WE(2) | PWM_BIPHASIC_CTRL0_WE(3) |
             PWM_BIPHASIC_CTRL0_WE(4) | PWM_BIPHASIC_CTRL0_WE(5) |
             PWM_BIPHASIC_CTRL0_WE(7);
    writel(ctrl0, pwm_dev->base + PWM_BIPHASIC_COUNTER_CTRL0);

    ctrl1 = (cfg->filter_enable ? PWM_BIPHASIC_CTRL1_FILTER_EN : 0) |
            PWM_BIPHASIC_CTRL1_FILTER_NUM(cfg->filter_number);
    ctrl1 |= PWM_BIPHASIC_CTRL1_WE(0) | PWM_BIPHASIC_CTRL1_WE(4) |
             PWM_BIPHASIC_CTRL1_WE(5) | PWM_BIPHASIC_CTRL1_WE(6) |
             PWM_BIPHASIC_CTRL1_WE(7) | PWM_BIPHASIC_CTRL1_WE(8) |
             PWM_BIPHASIC_CTRL1_WE(9) | PWM_BIPHASIC_CTRL1_WE(10);
    writel(ctrl1, pwm_dev->base + PWM_BIPHASIC_COUNTER_CTRL1);
    writel(cfg->timer_value, pwm_dev->base + PWM_BIPHASIC_COUNTER_TIMER_VALUE);

    /* Follow the TRM biphasic-counter flow: enable completion interrupt and
     * clear a stale status before enabling the counter. */
    writel(PWM_BIPHASIC_INT_STATUS | PWM_BIPHASIC_INT_WE,
           pwm_dev->base + PWM_INTSTS);
    writel(PWM_BIPHASIC_INT_STATUS | PWM_BIPHASIC_INT_WE,
           pwm_dev->base + PWM_INT_EN);

    pwm_dev->biphasic_on = 0;
    return RT_EOK;
}

static rt_err_t __pwm_biphasic_start(struct pwm_device *pwm_dev)
{
    rt_uint32_t reg;

    if (pwm_dev->biphasic_on)
        return RT_EOK;

    /* CTRL0 bit 0 is W1T: writing one starts the configured counter. */
    reg = PWM_BIPHASIC_CTRL0_WE(0) | PWM_BIPHASIC_CTRL0_EN;
    writel(reg, pwm_dev->base + PWM_BIPHASIC_COUNTER_CTRL0);
    pwm_dev->biphasic_on = 1;
    return RT_EOK;
}

static rt_err_t __pwm_biphasic_get_count(struct pwm_device *pwm_dev,
                                          struct rt_pwm_biphasic_count *count)
{
    if (!count)
        return -RT_EINVAL;

    count->count = (rt_int32_t)readl(pwm_dev->base +
                                     PWM_BIPHASIC_COUNTER_RESULT_VALUE_SYNC);
    return RT_EOK;
}

static rt_err_t __pwm_biphasic_set_callback(
    struct pwm_device *pwm_dev, const struct rt_pwm_biphasic_callback *cb)
{
    if (!cb)
        return -RT_EINVAL;

    pwm_dev->biphasic_callback = cb->callback;
    pwm_dev->biphasic_param = cb->param;
    return RT_EOK;
}

static void __pwm_enable(struct pwm_device *pwm_dev)
{
    rt_uint32_t reg;

    reg = readl(pwm_dev->base + PWM_ENABLE);
    reg |= PWM_ENABLE_CH0 | PWM_ENABLE_CH1 | PWM_ENABLE_WE(0) | PWM_ENABLE_WE(1);

    writel(reg, pwm_dev->base + PWM_ENABLE);
}

static void __pwm_disable(struct pwm_device *pwm_dev)
{
    rt_uint32_t reg;

    reg = readl(pwm_dev->base + PWM_ENABLE);
    reg &= ~(PWM_ENABLE_CH0 | PWM_ENABLE_CH1);
    reg |= PWM_ENABLE_WE(0) | PWM_ENABLE_WE(1);

    writel(reg, pwm_dev->base + PWM_ENABLE);
}

static rt_err_t __pwm_set_config(
    struct pwm_device *pwm_dev, struct rt_pwm_configuration *pwm_cfg)
{
    rt_uint32_t reg;
    rt_uint32_t period_ns;
    rt_uint32_t duty_ns;

    if (!pwm_cfg)
    {
        return (-RT_EINVAL);
    }

    if (pwm_cfg->pulse > pwm_cfg->period)
    {
        return (-RT_ERROR);
    }

    __pwm_disable(pwm_dev);

    period_ns = (rt_uint64_t)(pwm_dev->freq / 1000) * pwm_cfg->period / 1000000;
    duty_ns = (rt_uint64_t)(pwm_dev->freq / 1000) * pwm_cfg->pulse / 1000000;

    writel(period_ns, pwm_dev->base + PWM_PERIOD);
    writel(duty_ns, pwm_dev->base + PWM_DUTY);

    /* PWM_OFFSET is expressed in the same counter ticks as PWM_PERIOD. */
    writel((rt_uint64_t)period_ns * pwm_dev->phase / 360,
           pwm_dev->base + PWM_OFFSET);

    reg = readl(pwm_dev->base + PWM_CTRL);
    reg |= 1U | PWM_CTRL_CAPTURE_EN | PWM_CTRL_WE(1U) | PWM_CTRL_WE(5U);
    writel(reg, pwm_dev->base + PWM_CTRL);

    __pwm_enable(pwm_dev);

    return RT_EOK;
}

static rt_err_t __pwm_get_config(
    struct pwm_device *pwm_dev, struct rt_pwm_configuration *pwm_cfg)
{
    rt_uint32_t period_ns;
    rt_uint32_t duty_ns;

    if (!pwm_cfg)
    {
        return (-RT_EINVAL);
    }

    period_ns = 1000000 * readl(pwm_dev->base + PWM_PERIOD) /
        (rt_uint64_t)(pwm_dev->freq / 1000);
    duty_ns = 1000000 * readl(pwm_dev->base + PWM_DUTY) /
        (rt_uint64_t)(pwm_dev->freq / 1000);

    pwm_cfg->complementary = 0;
    pwm_cfg->dead_time = 0;
    pwm_cfg->phase = pwm_dev->phase;
    pwm_cfg->period = period_ns;
    pwm_cfg->pulse = duty_ns;

    return RT_EOK;
}

static void __pwm_set_period(struct pwm_device *pwm_dev, rt_uint32_t period)
{
    rt_uint32_t period_ns;

    period_ns = (rt_uint64_t)(pwm_dev->freq / 1000) * period / 1000000;
    writel(period_ns, pwm_dev->base + PWM_PERIOD);
}

static rt_err_t __pwm_set_phase(struct pwm_device *pwm_dev, rt_uint32_t phase)
{
    rt_uint64_t offset;
    rt_uint32_t max_valid_offset;
    rt_uint32_t period;
    rt_uint32_t duty;

    if (phase > 360)
    {
        return -RT_EINVAL;
    }

    __pwm_disable(pwm_dev);
    period = readl(pwm_dev->base + PWM_PERIOD);
    duty = readl(pwm_dev->base + PWM_DUTY);

    max_valid_offset = period - duty;

    offset = (rt_uint64_t)period * phase / 360;
    if(offset > max_valid_offset)
    {
        rt_kprintf("[WARN] phase %u exceed hardware limit, clamp offset from %llu to %u\n",
                   phase, offset, max_valid_offset);
        offset = max_valid_offset;
    }

    writel((rt_uint32_t)offset, pwm_dev->base + PWM_OFFSET);

    __pwm_enable(pwm_dev);

    return RT_EOK;
}



static void __pwm_set_pulse(struct pwm_device *pwm_dev, rt_uint32_t period)
{
    rt_uint32_t duty_ns;

    duty_ns = (rt_uint64_t)(pwm_dev->freq / 1000) * period / 1000000;
    writel(duty_ns, pwm_dev->base + PWM_DUTY);
}

static void pwm_irq_handler(int irq, void *param)
{
    struct pwm_device *pwm_dev = (struct pwm_device *)param;
    rt_uint32_t status;

    status = readl(pwm_dev->base + PWM_INTSTS);
    if (!(status & ((1U << 0) | (1U << 1) | PWM_BIPHASIC_INT_STATUS)))
    {
        return;
    }

    /* INTSTS uses one write-enable bit in [31:16] for every status bit. */
    if (status & ((1U << 0) | (1U << 1) | PWM_BIPHASIC_INT_STATUS))
    {
        writel((status & ((1U << 0) | (1U << 1) | PWM_BIPHASIC_INT_STATUS)) |
               ((status & ((1U << 0) | (1U << 1) | PWM_BIPHASIC_INT_STATUS)) << 16),
               pwm_dev->base + PWM_INTSTS);
    }

    if ((status & PWM_BIPHASIC_INT_STATUS) && pwm_dev->biphasic_callback)
    {
        pwm_dev->biphasic_callback(
            pwm_dev->biphasic_param,
            (rt_int32_t)readl(pwm_dev->base +
                              PWM_BIPHASIC_COUNTER_RESULT_VALUE));
    }

    if (pwm_dev->capture_callback)
    {
        pwm_dev->capture_callback(pwm_dev->capture_param,
                                  status & PWM_INT_CAPTURE_MASK,
                                  readl(pwm_dev->base + PWM_HPC),
                                  readl(pwm_dev->base + PWM_LPC),
                                  pwm_dev->freq);
    }
}

static rt_err_t __pwm_capture(
    struct pwm_device *pwm_dev, struct rt_pwm_configuration *pwm_cfg)
{
    rt_uint32_t reg;
    rt_uint32_t hpc, lpc;
    rt_uint32_t channel;

    if (!pwm_cfg)
    {
        return (-RT_EINVAL);
    }

    /* 首次调用启动持续捕获，中断保持使能 */
    if (!pwm_dev->capture_on)
    {
        channel = ((rt_uint32_t)pwm_dev->base >> 12) & 0x7;

        writel(PWM_INT_WE(PWM_INT_CAPTURE_MASK),
               pwm_dev->base + PWM_INT_EN);
        writel(PWM_INT_CAPTURE_MASK | PWM_INT_WE(PWM_INT_CAPTURE_MASK),
               pwm_dev->base + PWM_INTSTS);

        reg = readl(pwm_dev->base + PWM_CTRL);
        reg &= ~(PWM_CTRL_MODE_MASK | PWM_CTRL_CAPTURE_SEL(7) |
                 PWM_CTRL_CAPTURE_EN);
        reg |= 2U | PWM_CTRL_CAPTURE_SEL(channel) | PWM_CTRL_CAPTURE_EN;
        writel(reg | PWM_CTRL_WE(PWM_CTRL_MODE_MASK |
                                  PWM_CTRL_CAPTURE_SEL(7) |
                                  PWM_CTRL_CAPTURE_EN),
               pwm_dev->base + PWM_CTRL);

        writel(0, pwm_dev->base + PWM_HPC);
        writel(0, pwm_dev->base + PWM_LPC);

        writel(PWM_INT_WE(PWM_INT_CAPTURE_MASK),
               pwm_dev->base + PWM_INT_MASK);
        writel(PWM_INT_CAPTURE_MASK | PWM_INT_WE(PWM_INT_CAPTURE_MASK),
               pwm_dev->base + PWM_INT_EN);

        reg = readl(pwm_dev->base + PWM_ENABLE);
        reg |= (1U << 3) | PWM_ENABLE_WE(3);
        writel(reg, pwm_dev->base + PWM_ENABLE);

        pwm_dev->capture_on = 1;

        __pwm_enable(pwm_dev);
    }

    hpc = readl(pwm_dev->base + PWM_HPC);
    lpc = readl(pwm_dev->base + PWM_LPC);

    pwm_cfg->pulse = (rt_uint64_t)hpc * 1000000000ULL / pwm_dev->freq;
    pwm_cfg->period = (rt_uint64_t)(hpc + lpc) * 1000000000ULL / pwm_dev->freq;

    return RT_EOK;
}

static rt_err_t pwm_control(struct rt_device_pwm *pwm, int cmd, void *arg)
{
    rt_err_t err = RT_EOK;
    struct pwm_device *pwm_dev = rt_container_of(pwm, struct pwm_device, pwm);
    struct rt_pwm_configuration *pwm_cfg = (struct rt_pwm_configuration *)arg;

    switch (cmd)
    {
    case PWM_CMD_ENABLE: __pwm_enable(pwm_dev); break;

    case PWM_CMD_DISABLE: __pwm_disable(pwm_dev); break;

    case PWM_CMD_SET: err = __pwm_set_config(pwm_dev, pwm_cfg); break;

    case PWM_CMD_GET: err = __pwm_get_config(pwm_dev, pwm_cfg); break;

    case PWM_CMD_SET_PERIOD: __pwm_set_period(pwm_dev, pwm_cfg->period); break;

    case PWM_CMD_SET_PULSE: __pwm_set_pulse(pwm_dev, pwm_cfg->pulse); break;

    case PWM_CMD_SET_PHASE: err = __pwm_set_phase(pwm_dev, pwm_cfg->phase); break;

    case PWM_CMD_CAPTURE: err = __pwm_capture(pwm_dev, pwm_cfg); break;

    case PWM_CMD_SET_CAPTURE_CALLBACK:
    {
        struct pwm_capture_callback *cb = (struct pwm_capture_callback *)arg;
        pwm_dev->capture_callback = cb->callback;
        pwm_dev->capture_param = cb->param;
        break;
    }

    case PWM_CMD_BIPHASIC_CONFIG:
        err = __pwm_biphasic_config(pwm_dev,
                (const struct rt_pwm_biphasic_config *)arg);
        break;

    case PWM_CMD_BIPHASIC_START:
        err = __pwm_biphasic_start(pwm_dev);
        break;

    case PWM_CMD_BIPHASIC_GET_COUNT:
        err = __pwm_biphasic_get_count(pwm_dev,
                (struct rt_pwm_biphasic_count *)arg);
        break;

    case PWM_CMD_BIPHASIC_SET_CALLBACK:
        err = __pwm_biphasic_set_callback(pwm_dev,
                (const struct rt_pwm_biphasic_callback *)arg);
        break;

    default: err = (-RT_EINVAL); break;
    }

    return err;
}

static const struct rt_pwm_ops ops =
{
    .control = pwm_control,
};

static rt_err_t pwm_probe(struct rt_platform_device *pdev)
{
    rt_err_t err;
    const char *device_name;
    struct rt_ofw_node *np = pdev->parent.ofw_node;
    struct pwm_device *pwm_dev;

    pwm_dev = rt_calloc(1, sizeof(struct pwm_device));
    if (!pwm_dev)
    {
        err = (-RT_ENOMEM);
        goto err_exit;
    }

    pwm_dev->clk = rt_clk_find(np, "pwm");
    if (!pwm_dev->clk)
    {
        err = (-RT_ERROR);
        rt_kprintf("get pwm clk failed\n");
        goto err_exit;
    }
    pwm_dev->freq = rt_clk_get_rate(pwm_dev->clk);
    rt_clk_enable(pwm_dev->clk);

    pwm_dev->base = rt_ofw_iomap(np, 0);
    rt_ofw_prop_read_u32(np, "channel", &pwm_dev->channel);
    device_name = rt_ofw_node_full_name(np);

    err = rt_device_pwm_register(&pwm_dev->pwm, device_name, &ops, pwm_dev);
    if (err != RT_EOK)
    {
        rt_kprintf(
            "pwm device %s register failed, errcode:%d\n", device_name, err);
        goto err_exit;
    }

    pwm_dev->irq = rt_dm_dev_get_irq(&pdev->parent, 0);
    if (pwm_dev->irq > 0)
    {
        rt_hw_interrupt_install(pwm_dev->irq, pwm_irq_handler, pwm_dev, device_name);
        rt_hw_interrupt_umask(pwm_dev->irq);
    }

    rt_kprintf("pwm device: %s register success\n", device_name);

    return RT_EOK;

err_exit:
    if (pwm_dev)
    {
        rt_free(pwm_dev);
    }

    if (pwm_dev->clk)
    {
        rt_clk_disable(pwm_dev->clk);
    }

    return err;
}

static const struct rt_ofw_node_id pwm_ofw_ids[] =
{
    { .compatible = "rockchip,rk3506-pwm" },
    { /* sentinel */ }
};

static struct rt_platform_driver pwm_driver =
{
    .name = "pwm",
    .ids = pwm_ofw_ids,
    .probe = pwm_probe,
};

static int pwm_register(void)
{
    rt_platform_driver_register(&pwm_driver);

    return 0;
}
INIT_DEVICE_EXPORT(pwm_register);
