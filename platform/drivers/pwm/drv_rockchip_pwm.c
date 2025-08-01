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

struct pwm_device
{
    rt_uint32_t channel;
    rt_uint32_t freq;
    void *base;
    struct rt_clk *clk;
    struct rt_device_pwm pwm;
};

static void __pwm_enable(struct pwm_device *pwm_dev)
{
    rt_uint32_t reg;

    reg = readl(pwm_dev->base + PWM_ENABLE);
    reg |= (1 | (1 << 1)) | ((1 | (1 << 1)) << 16);

    writel(reg, pwm_dev->base + PWM_ENABLE);
}

static void __pwm_disable(struct pwm_device *pwm_dev)
{
    rt_uint32_t reg;

    reg = readl(pwm_dev->base + PWM_ENABLE);
    reg &= ~(1 | (1 << 1));
    reg |= ((1 | (1 << 1)) << 16);

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

    reg = readl(pwm_dev->base + PWM_CTRL);
    reg |= 1 | (1 << 16);
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
    pwm_cfg->phase = 0;
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

static void __pwm_set_pulse(struct pwm_device *pwm_dev, rt_uint32_t period)
{
    rt_uint32_t duty_ns;

    duty_ns = (rt_uint64_t)(pwm_dev->freq / 1000) * period / 1000000;
    writel(duty_ns, pwm_dev->base + PWM_DUTY);
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
