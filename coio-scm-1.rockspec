package = "coio"
version = "scm-1"
source = {
    url = "git://github.com/NelsonCrosby/coio"
}
description = {
    summary = "An asynchronous IO library for Lua, based on coroutines.",
    detailed = [[
        There are already libraries that do asynchronous IO in Lua, but they either
        use callbacks (ew), or are part of a full stack. Coio implements async IO
        using coroutines (which are much nicer than callbacks), and is independent of
        any framework or platform.
    ]],
    homepage = "https://github.com/NelsonCrosby/coio",
    license = "MIT"
}
dependencies = {
    "lua ~> 5.2"
}
build = {
    type = "builtin",
    modules = {
        coio = {
            sources = {
                "src/coio.c", "src/async.c", "src/loop.c",
                "src/util.c"
            },
            libraries = {"uv"}
        }
    }
}
