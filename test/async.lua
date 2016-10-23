local async = require('coio.async')
local loop = require('coio.loop')

TestAsync = {}

function TestAsync:testAsyncFunctionsExist()
    luaunit.assertIsFunction(async.async)
    luaunit.assertEquals(async[1], async.async)
    luaunit.assertEquals(getmetatable(async).__call, async.async)
end

function TestAsync:testAsyncFunctionCalled()
    local called = false
    local fn = async(function () called = true end)
    loop.create():run(function ()
        fn()
    end)
    luaunit.assertTrue(called)
end

function TestAsync:testAsyncFunctionDelayed()
    local mainCompleted = false
    local fn = async(function ()
        luaunit.assertTrue(mainCompleted)
    end)
    loop.create():run(function ()
        fn()
        mainCompleted = true
    end)
end


TestAwait = {}

function TestAwait:testAwaitFunctionsExist()
    luaunit.assertIsFunction(async.await)
    luaunit.assertEquals(async[2], async.await)
end

local await = async.await

function TestAwait:testAsyncFunctionCalled()
    local called = false
    local fn = async(function () called = true end)
    loop.create():run(function ()
        fn()
    end)
    luaunit.assertTrue(called)
end

function TestAwait:testAsyncFunctionAwaited()
    local mainCompleted = false
    local fn = async(function ()
        luaunit.assertFalse(mainCompleted)
    end)
    loop.create():run(function ()
        await(fn())
        mainCompleted = true
    end)
end

function TestAwait:testAwaitPropagatesError()
    local fn = async(function ()
        error('foo')
    end)
    loop.create():run(function ()
        luaunit.assertErrorMsgContains(': foo\n', await, fn())
    end)
end
