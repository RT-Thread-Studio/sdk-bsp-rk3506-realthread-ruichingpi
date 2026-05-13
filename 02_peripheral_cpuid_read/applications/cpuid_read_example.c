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

#define DBG_TAG "example.cpuinfo"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_err_t cpuid_read_example(void)
{
    rt_device_t dev;
    rt_uint8_t cpuid[16] = {0};
    rt_ssize_t len;
    int i;

    dev = rt_device_find("cpuinfo");
    if (dev == RT_NULL)
    {
        LOG_E("CPUINFO device not found");
        return -RT_ERROR;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDONLY) != RT_EOK)
    {
        LOG_E("Failed to open CPUINFO device");
        return -RT_ERROR;
    }

    len = rt_device_read(dev, 0, &cpuid, sizeof(cpuid));
    if (len != sizeof(cpuid))
    {
        LOG_E("Failed to read CPUINFO data");
        return -RT_ERROR;
    }

    LOG_I("CPUID read success!");

    rt_kprintf("CPUID: ");
    for (i = 0; i < sizeof(cpuid); i++)
    {
        rt_kprintf("%02X", cpuid[i]);
    }
    rt_kprintf("\n");

    return RT_EOK;
}
MSH_CMD_EXPORT(cpuid_read_example, cpuid read example);
