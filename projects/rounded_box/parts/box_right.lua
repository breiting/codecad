-- parts/box_base.lua
local B = require("util.box")

-- --- Parameters ---
local sz = param("size", 20)

local shell = B.centered(sz, sz, sz)

emit(shell)
