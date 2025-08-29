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
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <rtdevice.h>

#define DBG_TAG "example.gpio"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define LED 160

static int lua_led_on(lua_State *L)
{
    rt_pin_write(LED, PIN_LOW);
    return 1;
}

static int lua_led_off(lua_State *L)
{
    rt_pin_write(LED, PIN_HIGH);
    return 1;
}

static int lua_delay(lua_State *L)
{
    int num;
    num= lua_tointeger(L, 1);
    rt_thread_mdelay(num);
    return 1;
}

static const struct luaL_Reg mylib[]=
{
    {"led_on",lua_led_on},
    {"led_off",lua_led_off},
    {"delay",lua_delay},
    {NULL,NULL}
};

const char LUA_SCRIPT_GLOBAL[] =
{
    "num = 10;  "
    "off = 500; "
    "on = 500;  "
    "while num > 0 do "
    "    led_on(); "
    "    delay(on); "
    "    led_off(); "
    "    delay(off); "
    "    num = num - 1; "
    "end "
};

rt_err_t lua_led_loop(void)
{
    rt_pin_mode(LED, PIN_MODE_OUTPUT);
    LOG_I("LED loop");

    lua_State *L;
    L = luaL_newstate();
    luaopen_base(L);
    luaL_setfuncs(L, mylib, 0);
    luaL_dostring(L, LUA_SCRIPT_GLOBAL);
    lua_close(L);

    return RT_EOK;
}
MSH_CMD_EXPORT(lua_led_loop, lua_led_loop);


rt_err_t lua_led_turn_on(void)
{
    rt_pin_mode(LED, PIN_MODE_OUTPUT);
    LOG_I("turn led on");

    lua_State *L;
    L = luaL_newstate();
    luaopen_base(L);
    luaL_setfuncs(L, mylib, 0);
    luaL_dostring(L, "led_on()");
    lua_close(L);

    return RT_EOK;
}
MSH_CMD_EXPORT(lua_led_turn_on, Turn LED on via Lua);


rt_err_t lua_led_turn_off(void)
{
    rt_pin_mode(LED, PIN_MODE_OUTPUT);
    LOG_I("turn led off");

    lua_State *L;
    L = luaL_newstate();
    luaopen_base(L);
    luaL_setfuncs(L, mylib, 0);
    luaL_dostring(L, "led_off()");
    lua_close(L);

    return RT_EOK;
}
MSH_CMD_EXPORT(lua_led_turn_off, Turn LED off via Lua);


