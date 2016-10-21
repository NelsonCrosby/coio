# Coio - Async IO for Lua #

There are already libraries that do asynchronous IO in Lua, but they either
use callbacks (ew), or are part of a full stack. Coio implements async IO
using coroutines (which are much nicer than callbacks), and is independent of
any framework or platform.

It is implemented entirely in C, and based on the excellent libuv library.

During this development stage, I'm only supporting Linux (since that's what
I'm using). PRs for other platforms are still welcome.
