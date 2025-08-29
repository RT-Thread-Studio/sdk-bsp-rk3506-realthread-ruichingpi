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

#define UPGRADE_CTRL_EXECUTE 0x01

typedef enum
{
    UPGRADE_TGT_APP = 0,
    UPGRADE_TGT_DTB = 1,
} upgrade_target_t;

typedef struct
{
    upgrade_target_t target;
    const char *app_path;
    const char *dtb_path;
} upgrade_request_t;

#endif /* __APP_UPGRADE_H__ */
