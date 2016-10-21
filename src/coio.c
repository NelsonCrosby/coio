#include <lua.h>
#include <uv.h>


int luaopen_coio(lua_State *L)
{
    lua_createtable(L, 0, 1);
    lua_pushinteger(L, uv_version());
    lua_setfield(L, -2, "uv_version");
    return 1;
}
