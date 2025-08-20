-- parts/box_base.lua
local B = require("util.box")

-- --- Parameters ---
local sz = param("size", 30)

local shell = B.centered(sz, sz + 20, sz)

emit(shell)
