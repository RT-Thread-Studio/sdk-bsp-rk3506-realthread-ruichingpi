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
#include <drv_rockchip_pwm.h>

#define BIPHASIC_PWM_DEVICE "pwm@ff170000"

static int encoder_qy1503_example(void)
{
    struct rt_device_pwm *pwm;
    struct rt_pwm_biphasic_config config;
    struct rt_pwm_biphasic_count value;
    rt_err_t err;
    const int mode = PWM_BIPHASIC_MODE_4X;
    int i;

    pwm = (struct rt_device_pwm *)rt_device_find(BIPHASIC_PWM_DEVICE);
    if (!pwm)
    {
        rt_kprintf("PWM device %s not found\n", BIPHASIC_PWM_DEVICE);
        return -RT_ENOSYS;
    }

    config.mode = mode;
    config.continuous = RT_TRUE;
    config.filter_enable = RT_FALSE;
    config.filter_number = 0;
    config.timer_value = 0;

    err = rt_device_control(&pwm->parent, PWM_CMD_BIPHASIC_CONFIG, &config);
    if (err != RT_EOK)
    {
        rt_kprintf("biphasic config failed: %d\n", err);
        return err;
    }

    err = rt_device_control(&pwm->parent, PWM_CMD_BIPHASIC_START, RT_NULL);
    if (err != RT_EOK)
    {
        rt_kprintf("biphasic start failed: %d\n", err);
        return err;
    }

    rt_kprintf("biphasic mode %d started, sample for 10 seconds\n", mode);
    for (i = 0; i < 10; i++)
    {
        rt_thread_mdelay(1000);
        err = rt_device_control(&pwm->parent, PWM_CMD_BIPHASIC_GET_COUNT,
                                &value);
        if (err != RT_EOK)
        {
            rt_kprintf("read count failed: %d\n", err);
            return err;
        }
        rt_kprintf("[sync count=%d]\n", value.count);
    }

    rt_kprintf("test finished; counter remains enabled by hardware design\n");
    return RT_EOK;
}
MSH_CMD_EXPORT(encoder_qy1503_example, encoder_qy1503_example);
