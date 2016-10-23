local loop = require('coio.loop')

TestLoop = {}

function TestLoop:testRunFunction()
    local run = false
    local evl = loop.create()
    evl:run(function ()
        run = true
    end)
    luaunit.assertTrue(run)
end
