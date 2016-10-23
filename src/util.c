#include <lua.h>
#include <lauxlib.h>
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
        lua_insert(t, 1);
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
        int error_caught = 0;
        if (lua_isuserdata(t, -1)) {
            // u = touserdata(awt)
            lua_State *u = (lua_State *) lua_touserdata(t, -1);
            lua_pop(t, 3);  // Pop awt, ai, R
            // Put results on u
            lua_xmove(t, u, nresults);
            if (status != LUA_OK) {
                // There was an error; add traceback for justice
                const char *msg = lua_tostring(u, -1);
                luaL_traceback(u, t, msg, 1);
                lua_remove(u, -2);
            }
            // Push u to event loop
            coio_util_queue_thread(u);
            error_caught = 1;
        } else {
            lua_pop(t, 3);  // Pop awt, ai, R
            if (status == LUA_OK) {
                error_caught = 1;
            }
        }
        // Stop gc so thread doesn't disappear
        // before we get our traceback
        lua_gc(t, LUA_GCSTOP, 0);
        if (!error_caught) {
            // There's an unhandled error.
            // Get a traceback for justice,
            // then raise error on main thread.
            // T = REG[MAINTHREAD]
            lua_pushinteger(t, LUA_RIDX_MAINTHREAD);
            lua_gettable(t, LUA_REGISTRYINDEX);
            // L = tothread(T)
            lua_State *L = lua_tothread(t, -1);
            lua_pop(t, 1);  // Pop T
            // Generate traceback
            const char *msg = lua_tostring(t, -1);
            luaL_traceback(L, t, msg, 1);
            // Restart the gc
            lua_gc(t, LUA_GCRESTART, 0);
            // Raise error on main thread
            lua_error(L);
            return;
        } else {
            // Restart the gc
            lua_gc(t, LUA_GCRESTART, 0);
        } // end else for if (!error_caught)
    } // end thread done block
}


static void handle_close(uv_handle_t *h)
{
    lua_State *t = (lua_State *) h->data;

    // Let h be freed
    lua_pushlightuserdata(t, coio_loop_curidx);
    lua_gettable(t, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(t, (void *) h);
    lua_pushnil(t);
    lua_settable(t, -3);
    lua_pop(t, 1);  // Reset stack

    // Let gc free this thread
    // R = REG[curidx]
    lua_pushlightuserdata(t, coio_loop_curidx);
    lua_gettable(t, LUA_REGISTRYINDEX);
    // R[t] = nil
    lua_pushlightuserdata(t, (void *) t);
    lua_pushnil(t);
    lua_settable(t, -3);    // t is now only ref'd by itself
    lua_pop(t, 1);  // Pop R
}

static void timer_resume(uv_timer_t *h)
{
    lua_State *t = (lua_State *) h->data;

    int nargs = lua_status(t) == LUA_OK
        ? lua_gettop(t) - 1
        : 0;
    // Resume t
    coio_util_run_thread(t, nargs);
    // Close h
    uv_close((uv_handle_t *) h, handle_close);
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
