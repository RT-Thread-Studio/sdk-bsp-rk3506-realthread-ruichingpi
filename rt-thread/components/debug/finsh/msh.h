/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __MSH_H__
#define __MSH_H__

#include <rtthread.h>
#include <finsh_service.h>

int msh_exec(char *cmd, rt_size_t length);

#endif /* __MSH_H__ */
