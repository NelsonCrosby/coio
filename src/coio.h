#include <lua.h>

// module 'coio'
int luaopen_coio(lua_State *L);

// module 'coio.loop'
#define COIO_LOOP_TNAME     "coio.loop"
extern void *const coio_loop_curidx;
int luaopen_coio_loop(lua_State *L);
int coio_loop_create(lua_State *L);
int coio_loop_run(lua_State *L);
int coio_loop__gc(lua_State *L);

// module 'coio.async'
#define COIO_ASYNC_TNAME    "coio.async"
extern void *const coio_async_noauto;
int luaopen_coio_async(lua_State *L);
int coio_async(lua_State *L);
int coio_await(lua_State *L);
