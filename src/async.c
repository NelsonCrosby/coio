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
    int ctx, ok;
    switch (lua_getctx(L, &ctx)) {
    case LUA_OK:
        // Get rid of any extra arguments
        lua_pop(L, lua_gettop(L) - 1);
        // Ensure we're not going to run
        // head-first into major trouble
        // with the gc.
        // Check there is a running event loop.
        lua_pushlightuserdata(L, coio_loop_curidx);
        lua_gettable(L, LUA_REGISTRYINDEX);
        if (lua_isnil(L, -1)) {
            return luaL_error(L,
                    "await can only be called with a running event loop");
        }
        // Check the current thread
        // is an async function (on
        // the event loop's table).
        lua_pushlightuserdata(L, (void *) L);
        lua_gettable(L, -2);
        if (lua_isnil(L, -1)) {
            return luaL_error(L,
                    "await can only be called from an async function");
        } else {
            lua_pop(L, 2);  // Only needed these for verification
        }
        // Check there isn't already something awaiting
        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
        if (!lua_isnil(L, -2)) {
            return luaL_error(L,
                    "each thread can only have one other awaiting it");
        } else {
            lua_pop(L, 1);
        }
        // Set this thread as awaiting
        // on the async instance.
        lua_pushinteger(L, 1);
        lua_pushlightuserdata(L, (void *) L);
        lua_settable(L, -3);
        lua_pop(L, 1);  // Clear stack
        // Do an ioyield to ensure we don't
        // get resumed until we actually
        // have the results.
        lua_pushlightuserdata(L, coio_util_ioyield);
        return lua_yieldk(L, 1, ctx, coio_await);
    default:
        lua_remove(L, 1);
        // Pull the success indicator
        // from the stack
        ok = lua_toboolean(L, 1);
        lua_remove(L, 1);
        if (ok) {
            // Successful - return results
            return lua_gettop(L);
        } else {
            // Failure - re-raise error
            return lua_error(L);
        }
    }
}
