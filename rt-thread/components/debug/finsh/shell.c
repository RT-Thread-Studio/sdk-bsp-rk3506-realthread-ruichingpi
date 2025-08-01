/**
 * RT-Thread RuiChing
 *
 * COPYRIGHT (C) 2024-2025 Shanghai Real-Thread Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution.
 */
#include <finsh_service.h>
#include <service.h>
#include <shell.h>

const char *finsh_get_prompt(void)
{
    static char finsh_prompt[RT_CONSOLEBUF_SIZE + 1] = { 0 };
    rt_err_t ret;

    ret =
        service_control(finsh_service_get(), APP_FINSH_GET_PROMPT, (void *)finsh_prompt);
    if (ret != RT_EOK)
    {
        rt_kprintf("prompt get failed: %d\n", ret);
        return RT_NULL;
    }

    return finsh_prompt;
}

void finsh_set_echo(rt_uint32_t echo)
{
    rt_err_t ret;
    rt_uint32_t echo_value = echo;

    ret = service_control(
        finsh_service_get(), APP_FINSH_SET_ECHO, (void *)&echo_value);
    if (ret != RT_EOK)
    {
        rt_kprintf("echo set: %d\n", ret);
    }
}

rt_uint32_t finsh_get_echo(void)
{
    rt_err_t ret;
    rt_uint32_t echo = 0;

    ret =
        service_control(finsh_service_get(), APP_FINSH_GET_ECHO, (void *)&echo);
    if (ret != RT_EOK)
    {
        rt_kprintf("echo get failed: %d\n", ret);
        return 0;
    }

    return echo;
}