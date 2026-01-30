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

#define AMP_SHM_IOCTL_GET_INFO          0x40
#define AMP_SHM_IOCTL_KICK              0x41
#define AMP_SHM_IOCTL_WAIT              0x42
#define AMP_SHM_IOCTL_MAP               0x43
#define AMP_SHM_IOCTL_UNMAP             0x44
#define AMP_SHM_IOCTL_FLUSH_CACHE       0x45
#define AMP_SHM_IOCTL_INVALID_CACHE     0x46

#define AMP_SHM_REMOTE_EVENT            (1 << 0)

struct amp_shm_args
{
    void *vaddr;
    rt_uint32_t size;
    rt_uint32_t cache_offset;
    rt_uint32_t cache_len;
};


static void help(const char *prog)
{
    rt_kprintf("usage:\n");
    rt_kprintf("  %s <dev> send_notify\n", prog);
    rt_kprintf("  %s <dev> wait_notify\n", prog);
    rt_kprintf("  %s <dev> write <offset> <string>\n", prog);
    rt_kprintf("  %s <dev> read  <offset> <len>\n", prog);
    rt_kprintf("\nExamples:\n");
    rt_kprintf("  %s amp-shm send_notify\n", prog);
    rt_kprintf("  %s amp-shm wait_notify\n", prog);
    rt_kprintf("  %s amp-shm write 0 \"world\"\n", prog);
    rt_kprintf("  %s amp-shm read  0 16\n", prog);
}

static int amp_shm_test(int argc, char**argv)
{
#define __cmd_is(x, ...) if (!rt_strcmp(argv[2], x) __VA_ARGS__)

    const char *dev_name = RT_NULL;
    rt_device_t dev_test;
    struct amp_shm_args args = { 0 };
    rt_err_t result = -RT_ERROR;

    if(argc < 3)
        help(argv[0]);

    dev_name = argv[1];
    dev_test = rt_device_find(dev_name);

    if (!dev_test)
    {
        rt_kprintf("dev '%s' not found\n", dev_name);
        return 0;
    }

    if (rt_device_open(dev_test, 0))
    {
        rt_kprintf("dev '%s' open fail\n", dev_name);
        return 0;
    }


    __cmd_is("send_notify")
    {
        rt_device_control(dev_test, AMP_SHM_IOCTL_KICK, RT_NULL);
    }

    __cmd_is("wait_notify")
    {
        result = rt_device_control(dev_test, AMP_SHM_IOCTL_WAIT, RT_NULL);
        if(result != RT_EOK)
        {
            rt_kprintf("Interrupt wait failed");
        }
    }

    __cmd_is("write", && argc == 5)
    {
        rt_uint32_t size = 0;
        rt_uint32_t offset = 0;
        rt_uint8_t *vaddr = RT_NULL;
        char *str = RT_NULL;
        int i = 0;

        result = rt_device_control(dev_test, AMP_SHM_IOCTL_GET_INFO, &args);
        if (result != RT_EOK)
        {
            rt_kprintf("failed to retrieve shm info\n");
            return result;
        }

        size = args.size;

        result = rt_device_control(dev_test, AMP_SHM_IOCTL_MAP, &args);
        if (result != RT_EOK)
        {
            rt_kprintf("failed to map shm\n");
            return result;
        }

        vaddr = (rt_uint8_t *)args.vaddr;
        offset = (rt_uint32_t)atol(argv[3]);
        str = argv[4];

        if ((offset + rt_strlen(str)) > size)
        {
            rt_kprintf("data write exceeds shm size\n");
            goto out;
        }

        rt_memcpy((vaddr + offset), str, rt_strlen(str));

        args.cache_offset = offset;
        args.cache_len = rt_strlen(str);

        rt_device_control(dev_test, AMP_SHM_IOCTL_FLUSH_CACHE, &args);
        rt_device_control(dev_test, AMP_SHM_IOCTL_UNMAP, &args);
    }

    __cmd_is("read", && argc == 5)
    {
        rt_uint32_t size = 0;
        rt_uint32_t offset = 0;
        rt_uint32_t len = 0;
        rt_uint8_t *vaddr = RT_NULL;
        char *str = RT_NULL;
        int i;

        result = rt_device_control(dev_test, AMP_SHM_IOCTL_GET_INFO, &args);
        if (result != RT_EOK)
        {
            rt_kprintf("failed to retrieve shm info\n");
            return result;
        }

        size = args.size;

        result = rt_device_control(dev_test, AMP_SHM_IOCTL_MAP, &args);
        if (result != RT_EOK)
        {
            rt_kprintf("failed to map shm\n");
            return result;
        }

        vaddr = (rt_uint8_t *)args.vaddr;
        offset = (rt_uint32_t)atol(argv[3]);
        len = (rt_uint32_t)atol(argv[4]);

        if ((offset + len) > size)
        {
            rt_kprintf("read range exceeds shared memory size\n");
            goto out;
        }

        args.cache_offset = offset;
        args.cache_len = len;

        rt_device_control(dev_test, AMP_SHM_IOCTL_INVALID_CACHE, &args);

        for (i = 0; i < len; i++)
        {
            char c = *((char *)(vaddr + offset + i));
            rt_kprintf("[%2d] = %02x (%c)\n", i, c, c);
        }

        rt_device_control(dev_test, AMP_SHM_IOCTL_UNMAP, &args);
    }

    return 0;
out:
    rt_device_control(dev_test, AMP_SHM_IOCTL_UNMAP, &args);
    return 0;

#undef __cmd_is
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(amp_shm_test, amp shm test);
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(amp_shm_test, amp shm test)
#endif /* FINSH_USING_MSH */
#endif /* RT_USING_FINSH */

