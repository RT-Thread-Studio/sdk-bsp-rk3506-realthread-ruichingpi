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
#include <rtdevice.h>

#define PWM_DEV_NAME    "pwm@ff932000"
#define PWM_DEV_CHANNEL 0

#define INITIAL_PERIOD  500000
#define INITIAL_PULSE   50000
#define PULSE_STEP      5000

static void pwm_sample(void)
{
    struct rt_device_pwm *pwm_dev;
    rt_uint32_t period = INITIAL_PERIOD;
    rt_uint32_t pulse = INITIAL_PULSE;
    rt_bool_t increasing = RT_TRUE;

    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf(
            "PWM example failed! PWM Device %s not found!\n", PWM_DEV_NAME);
        return;
    }

    rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
    rt_pwm_enable(pwm_dev, PWM_DEV_CHANNEL);

    while (1)
    {
        rt_thread_mdelay(50);

        pulse += increasing ? PULSE_STEP : -PULSE_STEP;

        if (pulse >= period)
        {
            increasing = RT_FALSE;
            pulse = period;
        }
        else if (pulse == 0)
        {
            increasing = RT_TRUE;
        }

        rt_pwm_set(pwm_dev, PWM_DEV_CHANNEL, period, pulse);
    }
}

int main(void)
{
    rt_kprintf("RT-Thread APP PWM Demo\n");

    pwm_sample();

    return 0;
}
