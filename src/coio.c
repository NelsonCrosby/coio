#include <lua.h>
#include <uv.h>

#include "coio.h"


int luaopen_coio(lua_State *L)
{
    lua_createtable(L, 0, 1);

    lua_getglobal(L, "require");
    lua_pushstring(L, "coio.loop");
    lua_call(L, 1, 1);
    lua_setfield(L, -2, "loop");

    return 1;
}
