/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __COREDUMP_SERVER_H__
#define __COREDUMP_SERVER_H__

#include <rtthread.h>

#define COREDUMP_CTRL_SET_CONFIG 0x01

struct app_ram_region
{
    rt_uint32_t ram_addr;
    rt_uint32_t ram_size;
};
typedef struct app_ram_region app_ram_region_t;

typedef enum
{
    CD_STORAGE_RAM = 0,
    CD_STORAGE_FILE,
    CD_STORAGE_RAM_AND_FILE
}coredump_storage_type_t;

struct app_memory_regions
{
    void *data_start;
    void *data_end;
    void *bss_start;
    void *bss_end;
};
typedef struct app_memory_regions app_memory_regions_t;

struct coredump_config
{
    rt_bool_t enabled;
    rt_size_t max_size;
    coredump_storage_type_t storage_type;
    const char *save_path;
    app_memory_regions_t app_regions;
    volatile app_ram_region_t ram_region;
};

rt_err_t coredump_init(void);

#endif /* __COREDUMP_SERVER_H__ */
