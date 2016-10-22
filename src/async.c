#include <lua.h>
#include <lauxlib.h>
#include <uv.h>

#include "coio.h"


static luaL_Reg module[] = {
    {"async", coio_async},
    {"await", coio_await},
    {NULL, NULL}
};

int luaopen_coio_async(lua_State *L)
{
    // Create module
    lua_createtable(L, 2, 1);
    // Set functions in array part for unpacking
    lua_pushinteger(L, 1);
    lua_pushcfunction(L, coio_async);
    lua_settable(L, -3);
    lua_pushinteger(L, 2);
    lua_pushcfunction(L, coio_await);
    lua_settable(L, -3);
    // Set functions as fields
    luaL_setfuncs(L, module, 0);
    // Set async function as __call
    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, coio_async);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);

    return 1;
}


static int coio_async_call(lua_State *L)
{
    // Create thread
    lua_State *t = lua_newthread(L);
    lua_insert(L, 1);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 2);
    lua_xmove(L, t, lua_gettop(L) - 1);
    // Wrap thread
    lua_createtable(L, 0, 1);
    luaL_setmetatable(L, COIO_ASYNC_TNAME);
    lua_insert(L, -2);
    lua_setfield(L, -2, "thread");

    // Prevent thread from being gc'd
    // before we're done with it
    lua_pushlightuserdata(L, coio_loop_curidx);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, (void *) t);
    lua_pushvalue(L, -3);   // Pull async instance
    lua_settable(L, -3);    // Set R[t] = ai
    // stack: ..., ai, R
    // Queue thread
    coio_util_queue_thread(t);

    // Return async instance
    lua_pop(L, 1);  // stack: ..., ai
    return 1;
}

// Returns an 'async function'.
// Async functions are coroutines that run on the event loop.
int coio_async(lua_State *L)
{
    lua_pushcclosure(L, coio_async_call, 1);
    return 1;
}


int coio_await(lua_State *L)
{
    return 0;
}
