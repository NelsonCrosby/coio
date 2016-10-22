#include <lua.h>
#include <lauxlib.h>
#include <uv.h>

#include "coio.h"


static luaL_Reg module[] = {
    {"async", coio_async},
    {"await", coio_await},
    {NULL, NULL}
};
void *const coio_async_noauto = (void *) &module;

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


static void async_push(lua_State *t);
static void async_resume(lua_State *t, int nargs)
{
    int status = lua_resume(t, NULL, nargs);
    if (status == LUA_YIELD) {
        if (lua_touserdata(t, 1) != coio_async_noauto) {
            // Non-IO yield, need to push
            // thread back to event loop
            async_push(t);
        }
    } else {
        // Thread done
        if (status == LUA_OK) {
            lua_pushboolean(t, 1);
        } else {
            lua_pushboolean(t, 0);
        }
        int nresults = lua_gettop(t);
        // Get awaiting thread
        // R = REG[curidx]
        lua_pushlightuserdata(t, coio_loop_curidx);
        lua_gettable(t, LUA_REGISTRYINDEX);
        // ai = R[t]
        lua_pushlightuserdata(t, (void *) t);
        lua_gettable(t, -2);
        // awt = ai[1]
        lua_pushinteger(t, 1);
        lua_gettable(t, -2);
        if (lua_isuserdata(t, -1)) {
            // u = touserdata(awt)
            lua_State *u = (lua_State *) lua_touserdata(t, -1);
            lua_pop(t, 3);  // Pop awt, ai, R
            // Put results on u
            lua_xmove(t, u, nresults);
            // Push u to event loop
            async_push(u);
        }
        // Let gc free this thread
        lua_pushlightuserdata(t, coio_loop_curidx);
        lua_gettable(t, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(t, (void *) t);
        lua_pushnil(t);
        lua_settable(t, -3);    // t is now only ref'd by itself
    }
}

static void async_timer_resume(uv_timer_t *h)
{
    lua_State *t = (lua_State *) h->data;
    // Let h be freed
    lua_pushlightuserdata(t, coio_loop_curidx);
    lua_gettable(t, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(t, (void *) h);
    lua_pushnil(t);
    lua_settable(t, -3);
    lua_pop(t, 1);  // Reset stack

    // Resume t
    async_resume(t, 0);
}

static void async_push(lua_State *t)
{
    // Get current loop
    lua_pushlightuserdata(t, coio_loop_curidx);
    lua_gettable(t, LUA_REGISTRYINDEX);
    lua_getfield(t, -1, "uv_loop");
    uv_loop_t *loop = (uv_loop_t *) lua_touserdata(t, -1);
    lua_pop(t, 1);  // Don't need loop on stack
    // Queue thread to be called on event loop
    uv_timer_t *h = (uv_timer_t *) lua_newuserdata(t, sizeof(uv_timer_t));
    uv_timer_init(loop, h);
    h->data = (void *) t;   // Make thread accessible from callback
    uv_timer_start(h, async_timer_resume, 0, 0);
    // Prevent timer from being gc'd
    // Loop registry is already at -2
    lua_pushlightuserdata(t, (void *) h);
    lua_insert(t, -2);
    lua_settable(t, -3);
    lua_pop(t, 1);  // Remove loop reg to reset stack
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
    async_push(t);

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
