/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_ad7606.h"

#define AD7606_DEV_NAME             "adc0"
#define AD7606_DATA_P_CH            (10) /* 每个通道数据点数 */
#define AD7606_DATA_MAX_NUM         (AD7606_DATA_P_CH * AD7606_MAX_CHANNELS) /* 最大数据量 */
#define AD7606_USE_CHANNELS         (AD7606_CHANNEL(1) | AD7606_CHANNEL(2) | AD7606_CHANNEL(3) | \
                                     AD7606_CHANNEL(4) | AD7606_CHANNEL(5) | AD7606_CHANNEL(6) | \
                                     AD7606_CHANNEL(7) | AD7606_CHANNEL(8)) /* 具体使用的通道 注意通道编号需从1开始 */
#define AD7606_RANGE_IS_10V         (0)  /* 根据硬件RANGE引脚设定:1=±10V,0=±5V */
#define AD7606_REFER_VOLTAGE        (0)  /* 参考电压(mv) */

#if AD7606_RANGE_IS_10V
#define FULL_SCALE_MV               (10000) /* ±10V 量程 */
#else
#define FULL_SCALE_MV               (5000)  /* ±5V 量程 */
#endif

#define UART_NAME                   "uart3"
#define INT_TO_FLOAT(in1, out, pos, seg)                                                            \
{                                                                                                   \
    int len = 0;                                                                                    \
    if ((in1) < 0)                                                                                  \
    {                                                                                               \
        (len) = rt_sprintf((out) + (pos), "-%d.%03d%s", (-(in1)) / 1000, (-(in1)) % 1000, (seg));   \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        (len) = rt_sprintf((out) + (pos), "%d.%03d%s", (in1) / 1000, (in1) % 1000, (seg));          \
    }                                                                                               \
    (pos) += len;                                                                                   \
}

static rt_device_t _serial = RT_NULL;
static rt_adc_device_t _adc = RT_NULL;
static volatile rt_bool_t _run_flag = RT_FALSE;
static rt_thread_t _adc_thread = RT_NULL;

static void _adc_vol_show_thread(void *parameter)
{
    char out_str[64];
    rt_uint8_t ch_num = 0, pos = 0;
    int j = 0, i = 0;
    rt_err_t ret = RT_EOK;
    rt_int16_t raw_val[AD7606_DATA_MAX_NUM];
    rt_int32_t voltage[AD7606_DATA_MAX_NUM];
    struct rt_adc_dma_cfg adc_dma_cfg;
    struct serial_configure uart_cfg;

    uart_cfg.baud_rate = BAUD_RATE_115200;
    uart_cfg.data_bits = DATA_BITS_8;
    uart_cfg.stop_bits = STOP_BITS_1;
    uart_cfg.parity = PARITY_NONE;
    uart_cfg.invert = NRZ_NORMAL;
    uart_cfg.bufsz = 4096;
    uart_cfg.flowcontrol = 0;
    rt_device_control(_serial, RT_DEVICE_CTRL_CONFIG, &uart_cfg);
    rt_device_open(_serial, RT_DEVICE_OFLAG_RDWR);

    ret = rt_adc_enable(_adc, 0);
    if (ret != RT_EOK)
    {
        rt_kprintf("enable adc fail: %d\n", ret);
        _run_flag = RT_FALSE;
    }

    adc_dma_cfg.use_channels = AD7606_USE_CHANNELS;
    adc_dma_cfg.buf_len = sizeof(raw_val);
    adc_dma_cfg.dst_addr = (rt_uint16_t *)raw_val;
    rt_adc_control(_adc, RT_ADC_CMD_DMA_START, &adc_dma_cfg);
    if (ret != RT_EOK)
    {
        rt_kprintf("enable adc fail: %d\n", ret);
        _run_flag = RT_FALSE;
    }

    for (i = 1; i <= AD7606_MAX_CHANNELS; i++)
    {
        if (AD7606_USE_CHANNELS & AD7606_CHANNEL(i))
        {
            ch_num++;
        }
    }

    while (_run_flag)
    {
        for (j = 0; j < AD7606_DATA_P_CH; j++)
        {
            rt_memset(out_str, 0, sizeof(out_str));
            pos = 0;
            for (i = 0; i < ch_num; i++)
            {
                voltage[j * ch_num + i] = ((rt_int32_t)raw_val[j * ch_num + i] *
                                            (rt_int32_t)FULL_SCALE_MV) / 32767 + AD7606_REFER_VOLTAGE;
                if (i == ch_num - 1)
                {
                    INT_TO_FLOAT(voltage[j * ch_num + i], out_str, pos, "\r\n");
                }
                else
                {
                    INT_TO_FLOAT(voltage[j * ch_num + i], out_str, pos, ",");
                }
            }
            rt_device_write(_serial, 0, out_str, strlen(out_str));
        }

        rt_thread_delay(10);
    }

    if (_serial != RT_NULL)
    {
        rt_device_close(_serial);
    }
    if (_adc != RT_NULL)
    {
        rt_adc_control(_adc, RT_ADC_CMD_DMA_STOP, RT_NULL);
        rt_adc_disable(_adc, 0);
    }
    _serial = RT_NULL;
    _adc = RT_NULL;
    _adc_thread = RT_NULL;
}

int adc_vol_show(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;

    if (argc > 1)
    {
        if (rt_strcmp(argv[1], "--stop") == 0)
        {
            _run_flag = RT_FALSE;
            return RT_EOK;
        }
        else
        {
            _run_flag = RT_TRUE;
        }
    }
    else
    {
        _run_flag = RT_TRUE;
    }

    _serial = rt_device_find(UART_NAME);
    if (_serial == RT_NULL)
    {
        rt_kprintf("can't find %s\n", UART_NAME);
        goto exit;
    }

    _adc = (rt_adc_device_t)rt_device_find(AD7606_DEV_NAME);
    if (!_adc)
    {
        rt_kprintf("can't find %s\n", AD7606_DEV_NAME);
        goto exit;
    }

    if (_adc_thread != RT_NULL)
    {
        rt_kprintf("adc_vol_show is already running, use [adc_vol_show --stop] to stop\n");
        return RT_EOK;
    }

    _adc_thread = rt_thread_create("adc_vol", _adc_vol_show_thread, RT_NULL, 2048, 16, 10);
    if (_adc_thread == RT_NULL)
    {
        rt_kprintf("create thread failed\n");
        goto exit;
    }
    rt_thread_startup(_adc_thread);

    rt_kprintf("adc_vol_show is running\n");

    return ret;
exit:
    ret = -RT_ERROR;
    _run_flag = RT_FALSE;
    return ret;
}
MSH_CMD_EXPORT(adc_vol_show, ad7606 voltage read sample);
