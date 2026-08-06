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
#define DBG_TAG "example.fwversion"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define SMODULE_MAGIC  0x00707061

struct smodule_version_info
{
    rt_uint16_t kernel_ver_major;
    rt_uint16_t kernel_ver_minor;
    rt_uint16_t kernel_ver_patch;
    char kernel_build_date[12];
    char kernel_build_time[9];
    rt_uint16_t kernel_crc16;
    rt_uint16_t reserved;
    rt_uint32_t app_version_ident;
    rt_uint32_t app_size;
    rt_uint32_t app_crc;
    rt_uint32_t app_magic;
    char app_build[24];
};

static rt_err_t fw_version_example(void)
{
    rt_device_t dev;
    struct smodule_version_info info = {0};
    rt_ssize_t len;

    dev = rt_device_find("fwinfo");
    if (dev == RT_NULL)
    {
        LOG_E("fwinfo device not found");
        return -RT_ERROR;
    }

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDONLY) != RT_EOK)
    {
        LOG_E("Failed to open fwinfo device");
        return -RT_ERROR;
    }

    len = rt_device_read(dev, 0, &info, sizeof(info));
    if (len != sizeof(info))
    {
        LOG_E("Failed to read firmware version info");
        return -RT_ERROR;
    }

    LOG_I("firmware version read success!");

    rt_kprintf("=== kernel ===\n");
    rt_kprintf("version : %d.%d.%d\n",info.kernel_ver_major, info.kernel_ver_minor, info.kernel_ver_patch);
    rt_kprintf("build   : %s %s\n",info.kernel_build_date, info.kernel_build_time);
    rt_kprintf("=== app ===\n");
    rt_kprintf("build   : %s\n", info.app_build);
    rt_kprintf("size    : %u\n", info.app_size);
    rt_kprintf("crc     : 0x%08X\n", info.app_crc);

    return RT_EOK;
  }
MSH_CMD_EXPORT(fw_version_example, fw version read example);

