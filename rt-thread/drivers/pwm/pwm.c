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

static rt_err_t _pwm_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    struct rt_device_pwm *pwm = (struct rt_device_pwm *)dev;
    struct rt_pwm_configuration *configuration =
        (struct rt_pwm_configuration *)args;

    switch (cmd)
    {
    case PWMN_CMD_ENABLE: configuration->complementary = RT_TRUE; break;
    case PWMN_CMD_DISABLE: configuration->complementary = RT_FALSE; break;
    default:
        if (pwm->ops->control)
            result = pwm->ops->control(pwm, cmd, args);
        break;
    }

    return result;
}

/*
pos: channel
void *buffer: rt_uint32_t pulse[size]
size : number of pulse, only set to sizeof(rt_uint32_t).
*/
static rt_ssize_t _pwm_read(
    rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_err_t result = RT_EOK;
    struct rt_device_pwm *pwm = (struct rt_device_pwm *)dev;
    rt_uint32_t *pulse = (rt_uint32_t *)buffer;
    struct rt_pwm_configuration configuration = { 0 };

    configuration.channel = (pos > 0) ? (pos) : (-pos);

    if (pwm->ops->control)
    {
        result = pwm->ops->control(pwm, PWM_CMD_GET, &configuration);
        if (result != RT_EOK)
        {
            return 0;
        }

        *pulse = configuration.pulse;
    }

    return size;
}

/*
pos: channel
void *buffer: rt_uint32_t pulse[size]
size : number of pulse, only set to sizeof(rt_uint32_t).
*/
static rt_ssize_t _pwm_write(
    rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_err_t result = RT_EOK;
    struct rt_device_pwm *pwm = (struct rt_device_pwm *)dev;
    rt_uint32_t *pulse = (rt_uint32_t *)buffer;
    struct rt_pwm_configuration configuration = { 0 };

    configuration.channel = (pos > 0) ? (pos) : (-pos);

    if (pwm->ops->control)
    {
        result = pwm->ops->control(pwm, PWM_CMD_GET, &configuration);
        if (result != RT_EOK)
        {
            return 0;
        }

        configuration.pulse = *pulse;

        result = pwm->ops->control(pwm, PWM_CMD_SET, &configuration);
        if (result != RT_EOK)
        {
            return 0;
        }
    }

    return size;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops pwm_device_ops = { RT_NULL, RT_NULL, RT_NULL,
    _pwm_read, _pwm_write, _pwm_control };
#endif /* RT_USING_DEVICE_OPS */

rt_err_t rt_device_pwm_register(struct rt_device_pwm *device,
    const char *name,
    const struct rt_pwm_ops *ops,
    const void *user_data)
{
    rt_err_t result = RT_EOK;

    rt_memset(device, 0, sizeof(struct rt_device_pwm));

#ifdef RT_USING_DEVICE_OPS
    device->parent.ops = &pwm_device_ops;
#else
    device->parent.init = RT_NULL;
    device->parent.open = RT_NULL;
    device->parent.close = RT_NULL;
    device->parent.read = _pwm_read;
    device->parent.write = _pwm_write;
    device->parent.control = _pwm_control;
#endif /* RT_USING_DEVICE_OPS */

    device->parent.type = RT_Device_Class_PWM;
    device->ops = ops;
    device->parent.user_data = (void *)user_data;

    result = rt_device_register(&device->parent, name, RT_DEVICE_FLAG_RDWR);

    return result;
}

rt_err_t rt_pwm_enable(struct rt_device_pwm *device, int channel)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    /* Make it is positive num forever */
    configuration.channel = (channel > 0) ? (channel) : (-channel);

    /* If channel is a positive number (0 ~ n), it means using normal output pin.
     * If channel is a negative number (0 ~ -n), it means using complementary output pin. */
    if (channel > 0)
    {
        result = rt_device_control(
            &device->parent, PWMN_CMD_DISABLE, &configuration);
    }
    else
    {
        result =
            rt_device_control(&device->parent, PWMN_CMD_ENABLE, &configuration);
    }

    result = rt_device_control(&device->parent, PWM_CMD_ENABLE, &configuration);

    return result;
}

rt_err_t rt_pwm_disable(struct rt_device_pwm *device, int channel)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    /* Make it is positive num forever */
    configuration.channel = (channel > 0) ? (channel) : (-channel);

    /* If channel is a positive number (0 ~ n), it means using normal output pin.
     * If channel is a negative number (0 ~ -n), it means using complementary output pin. */
    if (channel > 0)
    {
        result = rt_device_control(
            &device->parent, PWMN_CMD_DISABLE, &configuration);
    }
    else
    {
        result =
            rt_device_control(&device->parent, PWMN_CMD_ENABLE, &configuration);
    }

    result =
        rt_device_control(&device->parent, PWM_CMD_DISABLE, &configuration);

    return result;
}

rt_err_t rt_pwm_set(struct rt_device_pwm *device,
    int channel,
    rt_uint32_t period,
    rt_uint32_t pulse)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    configuration.channel = (channel > 0) ? (channel) : (-channel);
    configuration.period = period;
    configuration.pulse = pulse;
    result = rt_device_control(&device->parent, PWM_CMD_SET, &configuration);

    return result;
}

rt_err_t rt_pwm_set_period(
    struct rt_device_pwm *device, int channel, rt_uint32_t period)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    configuration.channel = (channel > 0) ? (channel) : (-channel);
    configuration.period = period;
    result =
        rt_device_control(&device->parent, PWM_CMD_SET_PERIOD, &configuration);

    return result;
}

rt_err_t rt_pwm_set_pulse(
    struct rt_device_pwm *device, int channel, rt_uint32_t pulse)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    configuration.channel = (channel > 0) ? (channel) : (-channel);
    configuration.pulse = pulse;
    result =
        rt_device_control(&device->parent, PWM_CMD_SET_PULSE, &configuration);

    return result;
}

rt_err_t rt_pwm_set_dead_time(
    struct rt_device_pwm *device, int channel, rt_uint32_t dead_time)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    configuration.channel = (channel > 0) ? (channel) : (-channel);
    configuration.dead_time = dead_time;
    result = rt_device_control(
        &device->parent, PWM_CMD_SET_DEAD_TIME, &configuration);

    return result;
}

rt_err_t rt_pwm_set_phase(
    struct rt_device_pwm *device, int channel, rt_uint32_t phase)
{
    rt_err_t result = RT_EOK;
    struct rt_pwm_configuration configuration = { 0 };

    if (!device)
    {
        return -RT_EIO;
    }

    configuration.channel = (channel > 0) ? (channel) : (-channel);
    configuration.phase = phase;
    result =
        rt_device_control(&device->parent, PWM_CMD_SET_PHASE, &configuration);

    return result;
}

rt_err_t rt_pwm_get(
    struct rt_device_pwm *device, struct rt_pwm_configuration *cfg)
{
    rt_err_t result = RT_EOK;

    if (!device)
    {
        return -RT_EIO;
    }

    result = rt_device_control(&device->parent, PWM_CMD_GET, cfg);

    return result;
}

rt_err_t rt_pwm_capture(
    struct rt_device_pwm *device, struct rt_pwm_configuration *cfg)
{
    rt_err_t result = RT_EOK;

    if (!device)
    {
        return -RT_EIO;
    }

    result = rt_device_control(&device->parent, PWM_CMD_CAPTURE, cfg);

    return result;
}
