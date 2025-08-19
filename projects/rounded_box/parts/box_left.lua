-- parts/box_base.lua
local B = require("util.box")
local T = require("util.transform")

local sz = 6
local shell = B.centered(sz, sz, sz)

local hole = cylinder(2, sz)

shell = difference(shell, hole)
shell = T.move_x(shell, 10)

emit(shell)
