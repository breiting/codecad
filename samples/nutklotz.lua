-- Nutklotz
-- units mm

local B = require("util.box")
local F = require("util.func")
local T = require("util.transform")
local Draw = require("util.drawing")

local width = 30
local length = 50
local height_total = 12
local height_bottom = 7
local height_middle = 3
local radius = 100
local N = 50

local hole_d = 6.5

local recess1 = 14

local function make_wedge(L, H)
	local shape = T.move_x(box(width, L, H), -width / 2)
	local arc = F.sample_arc(radius, width, N)
	local tip = T.rot_z(T.move_x(extrude(F.face_xy(arc, true), H), L), 90)
	return union(shape, tip)
end

local wedge1 = make_wedge(length, height_bottom)
local wedge2 = T.move_z(make_wedge(length - recess1, height_middle), height_bottom)

local wedge_height = height_total - height_bottom - height_middle
local wedge3 = T.move_x(T.rot_y(wedge(wedge_height, length - recess1, width, 0), -90), width / 2)
wedge3 = T.move_z(wedge3, height_bottom + height_middle)

local shape = union(wedge1, wedge2, wedge3)

local hole1 = T.move_y(cylinder(hole_d, height_total), length / 2)
local hole2 = T.move_z(T.move_y(cylinder(hole_d * 2, 3), length / 2), height_total - 3)

local holes = union(hole1, hole2)

shape = difference(shape, holes)

Draw.top_view_svg(shape, "out/test.svg", { scale = 1.0, margin = 50, dim_overall = true })

emit(shape)
