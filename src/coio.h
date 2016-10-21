#include <lua.h>

// module 'coio'
int luaopen_coio(lua_State *L);

// module 'coio.loop'
#define COIO_LOOP_TNAME     "coio.loop"
int luaopen_coio_loop(lua_State *L);
int coio_loop_create(lua_State *L);
int coio_loop_run(lua_State *L);
int coio_loop__gc(lua_State *L);
