/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#ifndef __SHELL_H__
#define __SHELL_H__

#include <rtthread.h>
#include <finsh.h>

#define FINSH_PROMPT finsh_get_prompt()

const char *finsh_get_prompt(void);

void finsh_set_echo(rt_uint32_t echo);

rt_uint32_t finsh_get_echo(void);

#endif /* __SHELL_H__ */
