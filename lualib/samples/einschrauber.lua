-- Einschrauber für Ringisolatoren
-- https://www.stallbedarf24.de/ako-einschrauber-fuer-alle-ringisolatoren-und-maxi-tape-isolator
--
--
--
local T = require("util.transform")
local B = require("util.box")
local sketch = require("util.sketch")

local cyl_h = 45
local cone_h = 20
local od = 46
local od_cone = 15.4
local id = 41.4
local slot1 = 10.5
local slot2 = 15.2

local thu_h = 35.4

local inner_h = 45
local hex_m = 8
local hex_h = 10

local function body()
	local cy = cylinder(od, cyl_h)
	local co = T.move_z(cone(od, od_cone, cone_h), cyl_h)
	local s = union(cy, co)
	return fillet(s, 0.5)
end

local function tri(x, y, ang)
	local s = sketch.begin(0, 0):line_to(5.8, 0):line_to(0, 5.8):close()
	local res = extrude(s:face(), 24.8)
	res = T.rot_z(res, ang)
	return translate(res, x, y, 0)
end

local b_thu = B.centered(slot1, od, thu_h)
local b_s = T.move_z(B.centered(slot1, id, inner_h - thu_h), thu_h)
local b_b = B.centered(id, slot2, inner_h)

local shape = body()

local nut = T.move_z(hex_prism(hex_m, hex_h), cyl_h + cone_h - hex_h)

shape = difference(shape, b_thu)
shape = difference(shape, b_s)
shape = difference(shape, b_b)
shape = difference(shape, nut)

shape = difference(shape, tri(9, 11, 0))
shape = difference(shape, tri(-9, 11, 90))
shape = difference(shape, tri(9, -11, -90))
shape = difference(shape, tri(-9, -11, 180))

-- optimize
shape = difference(shape, translate(B.centered(15, 15, 15), 0, -20, cyl_h + 2))
shape = difference(shape, translate(B.centered(15, 15, 15), 0, 20, cyl_h + 2))

emit(shape)
