/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <service.h>
#include <coredump_server.h>

struct service_core *cd_service = RT_NULL;
static struct coredump_config g_coredump_cfg;

static coredump_storage_type_t _get_storage_type(void)
{
#if defined(COREDUMP_STORAGE_RAM)
    return CD_STORAGE_RAM;
#elif defined(COREDUMP_STORAGE_FILE)
    return CD_STORAGE_FILE;
#else
    return CD_STORAGE_RAM_AND_FILE;
#endif
}

static app_memory_regions_t _get_app_memory_regions(void)
{
    extern char __data_start[], __data_end[];
    extern char __bss_cd_start[], __bss_cd_end[];

    app_memory_regions_t regions;

    regions.data_start = __data_start;
    regions.data_end = __data_end;
    regions.bss_start = __bss_cd_start;
    regions.bss_end = __bss_cd_end;

    return regions;
}

static rt_err_t coredump_service_set_config(const struct coredump_config *config)
{
    if ((!config) || (!config->app_regions.data_start) ||
        (!config->app_regions.data_end) || (!config->app_regions.bss_start) ||
        (!config->app_regions.bss_end))
    {
        return (-RT_EINVAL);
    }

    return service_control(cd_service, COREDUMP_CTRL_SET_CONFIG, (void*)config);
}

void app_coredump_load_config(void)
{
    rt_err_t ret;

    g_coredump_cfg.enabled = RT_TRUE;
    g_coredump_cfg.max_size = COREDUMP_MAX_SIZE_KB * 1024;
    g_coredump_cfg.storage_type = _get_storage_type();
    g_coredump_cfg.save_path = COREDUMP_FILE_SAVE_PATH;
    g_coredump_cfg.app_regions = _get_app_memory_regions();

    ret = coredump_service_set_config(&g_coredump_cfg);
    if (ret != RT_EOK)
    {
        rt_kprintf("coredump_service_set_config failed\n");
        return;
    }
}

rt_err_t coredump_init(void)
{
    cd_service = service_find("coredump_service");
    if (!cd_service)
    {
        rt_kprintf("service 'coredump_service' not found.\n");
        return (-RT_ERROR);
    }

    app_coredump_load_config();

    return RT_EOK;
}
