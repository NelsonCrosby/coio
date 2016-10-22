#include <lua.h>
#include <uv.h>

#include "coio.h"


static int _ioyield;
void *const coio_util_ioyield = (void *) &_ioyield;


void coio_util_run_thread(lua_State *t, int nargs)
{
    int status = lua_resume(t, NULL, nargs);
    if (status == LUA_YIELD) {
        if (lua_touserdata(t, 1) != coio_util_ioyield) {
            // Non-IO yield, need to push
            // thread back to event loop
            coio_util_queue_thread(t);
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
            coio_util_queue_thread(u);
        }
        // Let gc free this thread
        lua_pushlightuserdata(t, coio_loop_curidx);
        lua_gettable(t, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(t, (void *) t);
        lua_pushnil(t);
        lua_settable(t, -3);    // t is now only ref'd by itself
    }
}


static void timer_resume(uv_timer_t *h)
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
    coio_util_run_thread(t, 0);
}

void coio_util_queue_thread(lua_State *t)
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
    uv_timer_start(h, timer_resume, 0, 0);
    // Prevent timer from being gc'd
    // Loop registry is already at -2
    lua_pushlightuserdata(t, (void *) h);
    lua_insert(t, -2);
    lua_settable(t, -3);
    lua_pop(t, 1);  // Remove loop reg to reset stack
}
