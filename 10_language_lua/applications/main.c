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

int main(void)
{
    rt_kprintf("Hello, lua\n");

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dostring(L, "print('Hello, RT-Thread!123456789')") != LUA_OK)
    {
        rt_kprintf("Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1); 
    }

    lua_close(L);

    return 0;
}
