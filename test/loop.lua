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

function TestLoop:testErrorsPropagate()
    local evl = loop.create()
    luaunit.assertErrorMsgContains(
        ": foo\nstack traceback:",
        loop.run, evl,
        function () error('foo') end
    )
end

function TestLoop:testErrorsOnCallingThread()
    local evl1 = loop.create()
    evl1:run(function ()
        local evl2 = loop.create()
        luaunit.assertError(
            ": foo\nstack traceback:",
            loop.run, evl2,
            function () error('foo') end
        )
    end)
end
