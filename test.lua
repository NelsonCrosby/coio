local loop = require('coio.loop')
local async, await = unpack(require('coio.async'))

local one = async(function ()
    return 1
end)

local addone = async(function (a)
    return 1 + await(a)
end)

local evl = loop.create()
evl:run(function ()
    local a = one()
    local b = addone(a)
    local c = addone(a)
end)
