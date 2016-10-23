luaunit = require('luaunit')

tests = {
    'async',
    'loop'
}
for _, test in ipairs(tests) do
    require('test.' .. test)
end

os.exit(luaunit.LuaUnit.run())
