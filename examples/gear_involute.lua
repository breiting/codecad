-- examples/gear_involute.lua
local z = param("z", 24)
local m = param("m", 2.5)
local th = param("th", 8)
local bore = param("bore", 6)
local pdeg = param("pressure", 20)

local g = gear_involute(z, m, th, bore, pdeg)
emit(chamfer(g, math.min(0.2 * m, 0.6)))
