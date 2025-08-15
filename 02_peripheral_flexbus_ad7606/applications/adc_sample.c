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

#define ADC_DEV_NAME        "adc0"
#define ADC_CH              5
#define REFER_VOLTAGE       2500 /* 参考电压2.5v */

#define AD7606_RANGE_IS_10V 0 /* 根据硬件RANGE引脚设定：1=±10V，0=±5V */

#if AD7606_RANGE_IS_10V
#define FULL_SCALE_MV 10000 /* ±10V 量程 */
#else
#define FULL_SCALE_MV 5000 /* ±5V 量程 */
#endif

static int adc_vol_sample(int argc, char *argv[])
{
    rt_adc_device_t adc = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (!adc)
    {
        rt_kprintf("can't find %s\n", ADC_DEV_NAME);
        return -RT_ERROR;
    }

    rt_err_t ret = rt_adc_enable(adc, ADC_CH);
    if (ret != RT_EOK)
    {
        rt_kprintf("enable ch%d fail: %d\n", ADC_CH, ret);
        return ret;
    }

    rt_int16_t raw_val = rt_adc_read(adc, ADC_CH);
    rt_int32_t mv = ((rt_int32_t)raw_val * (rt_int32_t)FULL_SCALE_MV) / 32767;

    mv += REFER_VOLTAGE;

    if (mv >= 0)
    {
        rt_kprintf("ADC Channel %d: %d.%03d V\n", ADC_CH, mv / 1000, mv % 1000);
    }
    else
    {
        rt_kprintf(
            "ADC Channel %d: -%d.%03d V\n", ADC_CH, (-mv) / 1000, (-mv) % 1000);
    }

    rt_adc_disable(adc, ADC_CH);

    return RT_EOK;
}
MSH_CMD_EXPORT(adc_vol_sample, ad7606 voltage read sample);
