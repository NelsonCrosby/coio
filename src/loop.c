#include <lua.h>
#include <lauxlib.h>
#include <uv.h>

#include "coio.h"


static luaL_Reg module[] = {
    {"create", coio_loop_create},
    {"run", coio_loop_run},
    {NULL, NULL}
};

int luaopen_coio_loop(lua_State *L)
{
    lua_createtable(L, 0, 2);
    luaL_setfuncs(L, module, 0);
    return 1;
}


int coio_loop_create(lua_State *L)
{
    // Create the event loop
    uv_loop_t *loop = (uv_loop_t *) lua_newuserdata(L, sizeof(uv_loop_t));
    // Create metatable
    luaL_newmetatable(L, COIO_LOOP_TNAME);
    // gc event
    lua_pushcfunction(L, coio_loop__gc);
    lua_setfield(L, -2, "__gc");
    // index event
    lua_createtable(L, 0, 2);
    luaL_setfuncs(L, module + 1 /* Skip "create" */, 0);
    lua_setfield(L, -2, "__index");
    // Set metatable on userdatum
    lua_setmetatable(L, -2);
    // Finally, initialize the event loop
    uv_loop_init(loop);
    return 1;
}


// NOTE: This function is _blocking_.
int coio_loop_run(lua_State *L)
{
    uv_loop_t *loop = (uv_loop_t *) luaL_checkudata(L, 1, COIO_LOOP_TNAME);
    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}


int coio_loop__gc(lua_State *L)
{
    uv_loop_t *loop = (uv_loop_t *) lua_touserdata(L, 1);
    // TODO: Handle UV_EBUSY
    // The way to do that would probably be to error. I don't see any reason
    // that the GC should collect this object if the loop is still running.
    uv_loop_close(loop);
    return 0;
}
