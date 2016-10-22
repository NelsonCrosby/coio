local loop = require('coio.loop')
local async, await = unpack(require('coio.async'))

local evl = loop.create()
evl:run(function ()
    print('Hello, World!')
    -- Should be delayed until after the
    -- current function completes
    async(function () print('world') end)()
    print('hello')
end)
