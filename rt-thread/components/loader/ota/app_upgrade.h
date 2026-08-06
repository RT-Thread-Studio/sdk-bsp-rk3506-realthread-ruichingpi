/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __APP_UPGRADE_H__
#define __APP_UPGRADE_H__

#include <rtthread.h>

#define UPGRADE_CTRL_EXECUTE             0x01
#define UPGRADE_CTRL_CLEAR_APP_BOOT_FLAG 0x02
#define UPGRADE_CTRL_UPDATE_APP_BACK     0x03

typedef enum
{
    UPGRADE_TGT_APP = 0,
    UPGRADE_TGT_DTB = 1,
} upgrade_target_t;

typedef struct upgrade_request
{
    upgrade_target_t target;

    const char *app_path;
    const char *app_back_path;
    const char *dtb_path;

    rt_uint8_t *app_dst_base;
    rt_size_t app_dst_max;

    rt_uint8_t *dtb_dst_base;
    rt_size_t dtb_dst_max;

} upgrade_request_t;
rt_err_t app_upgrade_boot_ok(void);

#endif /* __APP_UPGRADE_H__ */
