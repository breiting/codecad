-- examples/centered_boxes.lua
local box = require("util.box")
local T = require("util.transform")

local sx = 60
local sy = 40
local sz = 20
local thickness = 2
local outer = box.centered(sx, sy, sz)                                              -- centered in XY
local inner = T.move_z(box.centered(sx - thickness, sy - thickness, sz), thickness) -- also centered
local shell = difference(outer, inner)                                              -- clean: no manual translate needed

-- move the whole result so the base sits on Z=0:
-- shell = translate(center_z(shell), 0, 0, 0)

emit(shell)
