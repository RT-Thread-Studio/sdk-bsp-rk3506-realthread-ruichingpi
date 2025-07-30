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
#include "drv_saradc.h"

#define DBG_TAG "example.saradc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void saradc_example(char argc, char *argv[])
{
    rt_uint8_t channel = 0;
    rt_uint32_t voltage;
    rk_saradc *saradc;

    if (argc != 2)
    {
        LOG_W("Usage: saradc_example <channel>, <channel> range is 0~3.\n");
        return;
    }
    else
    {
        channel = atoi(argv[1]);
    }

    saradc = saradc_init();
    saradc_start_channel(saradc, channel);
    voltage = saradc_to_voltage(saradc, channel);
    LOG_I("voltage: %d mV\n", voltage);
}
MSH_CMD_EXPORT(saradc_example, saradc_example);
