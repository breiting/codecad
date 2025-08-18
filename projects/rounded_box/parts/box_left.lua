-- parts/box_base.lua
local B = require("util.box")
local T = require("util.transform")

local sz = 60
local shell = B.centered(sz, sz, sz)

local hole = cylinder(8, sz)

shell = difference(shell, hole)

emit(shell)
save_stl(shell, "out/box_left.stl")
