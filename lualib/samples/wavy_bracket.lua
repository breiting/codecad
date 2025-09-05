-- wavy_bracket.lua
local func = require("util.func")
local box = require("util.box")
local T = require("util.transform")

local w = param("w", 80)
local h_base = param("h_base", 20)
local amp = param("amp", 1)
local freq = param("freq", 10)
local thick = param("thick", 20)

local f = function(x)
	return h_base + amp * math.sin(2 * math.pi * freq * (x / w))
end

local face = func.rect_with_func_top(0, w, 0, h_base, f, 300)
local solid = extrude(face, thick)

solid = T.rot_x(solid, 90)

solid = center_xy(solid)

local b = box.centered(w - 4, thick - 4, h_base - 5)

solid = difference(solid, b)
emit(solid)
