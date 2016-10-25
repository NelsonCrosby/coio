#include <lua.h>

// ## Utils ##

// Index indicating that a yield was
// caused by an IO function
extern void *const coio_util_ioyield;
// Resumes the passed thread and
// manages yields and awaits
void coio_util_run_thread(lua_State *t, int nargs);
// Enqueues the passed thread
// in the current event loop
void coio_util_queue_thread(lua_State *t);


// ## module `coio` ##

// Loads the `coio` base module
int luaopen_coio(lua_State *L);


// ## module `coio.loop` ##

// The name for the event loop metamethod
#define COIO_LOOP_TNAME     "coio.loop"
// Index acting as the key to the
// current event loop data on the
// Lua registry table
extern void *const coio_loop_curidx;
// Loads the `coio.loop` module.
// This module provides the base event
// loop type.
int luaopen_coio_loop(lua_State *L);
// Lua method for creating new
// event loops. Usually only used
// once. An event loop does not
// become "current" until it runs.
int coio_loop_create(lua_State *L);
// Lua method for running event
// loops. This could be used either
// as `loop.run(evl, f)` or
// `evl:run(f)`. The function
// argument is the "entry point" -
// it is the first thing run in
// the context of this event loop.
int coio_loop_run(lua_State *L);
// Lua method for responding to
// the gc event. Cleans up the
// event loop structure.
int coio_loop__gc(lua_State *L);


// ## module `coio.async` ##

// The name for the async instance metatable.
#define COIO_ASYNC_TNAME    "coio.async"
// Loads the `coio.async` module. This
// module provides the `async()` and
// `await()` functions. It allows
// multiple reference styles, such as
// `async, await = unpack(require('coio.async'))`
// or `async = require('coio.async') ; await = async.await`.
int luaopen_coio_async(lua_State *L);
// Lua `async()` function. Creates
// an "async function" from the
// passed function argument. Async
// functions, when called, enqueue
// the underlying function on the
// event loop and return a reference
// to the function instance for use
// in `await()`.
int coio_async(lua_State *L);
// Lua `await()` function. Takes
// an async function and waits
// until the function returns.
// It returns anything returned
// from the thread, or re-triggers
// any errors.
int coio_await(lua_State *L);
