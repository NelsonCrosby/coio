local loop = require('coio.loop')
local async, await = unpack(require('coio.async'))

local apr = async(function (...)
    print(...)
end)

local testgood = async(function ()
    print('1')
    await(apr('2'))
    -- Should be delayed until after the
    -- current function completes
    apr('4')
    print('3')
end)

local testerr = async(function ()
    error('this is an error')
end)

local evl = loop.create()
evl:run(function ()
    await(testgood())
    -- This call should result
    -- in an error with
    -- three stack tracebacks
    await(testerr())
end)
