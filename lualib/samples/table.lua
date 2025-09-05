-- A nice dining table with extension element
-- unit cm
--
local B = require("util.box")
local T = require("util.transform")

-- CONFIG begin
local top_width = 150
local top_height = 80
local top_thickness = 4
local table_height = 75
local leg_dim = 7
local beam_recess = 3

local rail_height = 8
local rail_thickness = 2

local attach_width = 50
local attach_height = 80
local attach_rail_width = 2
local attach_rail_height = 3
local attach_rail_distance = 35

local chamfer_sz = 0.2
-- CONFIG end

local leg_height = table_height - top_thickness

-- Seitenteil Unterstell
local function end_rail()
	local rail =
		chamfer(B.centered(top_height - 2 * beam_recess - 2 * leg_dim, rail_height, rail_thickness), chamfer_sz)
	local b1 = T.move_x(B.centered(attach_rail_width, attach_rail_height, rail_thickness), -attach_rail_distance / 2)
	b1 = T.move_y(b1, rail_height / 2 - attach_rail_height / 2)
	local b2 = T.move_x(B.centered(attach_rail_width, attach_rail_height, rail_thickness), attach_rail_distance / 2)
	b2 = T.move_y(b2, rail_height / 2 - attach_rail_height / 2)
	return difference(difference(rail, b1), b2)
end

-- Längsseite Unterstell
local function side_rail()
	return chamfer(B.centered(top_width - 2 * beam_recess - 2 * leg_dim, rail_height, rail_thickness), chamfer_sz)
end

-- Fuss
local function beam()
	return chamfer(B.centered(leg_dim, leg_dim, leg_height), chamfer_sz)
end

-- Ansteckplaatte
local function attach_top()
	return B.centered(attach_width, attach_height, top_thickness)
end

-- Construction
--
-- Tischplatte
local function make_top()
	return chamfer(T.move_z(B.centered(top_width, top_height, top_thickness), table_height - top_thickness), chamfer_sz)
end

-- Ansteckplatte
local function make_attach()
	return T.move_z(B.centered(top_width, top_height, top_thickness), table_height)
end

-- Füsse
local function make_beams(leg)
	local tx = top_width / 2
	local ty = top_height / 2
	local lz = leg_dim / 2
	local br = beam_recess
	local l1 = translate(beam(), tx - lz - br, -ty + lz + br, 0)
	local l2 = translate(beam(), tx - lz - br, ty - lz - br, 0)
	local l3 = translate(beam(), -tx + lz + br, -ty + lz + br, 0)
	local l4 = translate(beam(), -tx + lz + br, ty - lz - br, 0)
	return union(l1, l2, l3, l4)
end

-- Seitenbretter
local function make_end_rails()
	local r1 = T.rot_z(T.rot_x(end_rail(), 90), 90)
	r1 = T.move_z(r1, table_height - top_thickness - rail_height / 2)
	r1 = T.move_x(r1, -top_width / 2 + beam_recess)

	local r2 = T.rot_z(T.rot_x(end_rail(), 90), -90)
	r2 = T.move_z(r2, table_height - top_thickness - rail_height / 2)
	r2 = T.move_x(r2, top_width / 2 - beam_recess)
	return union(r1, r2)
end

-- Längsbretter
local function make_side_rails()
	local r1 = T.rot_x(side_rail(), 90)
	r1 = T.move_z(r1, table_height - top_thickness - rail_height / 2)
	r1 = T.move_y(r1, -top_height / 2 + beam_recess + rail_thickness)

	local r2 = T.rot_x(side_rail(), 90)
	r2 = T.move_z(r2, table_height - top_thickness - rail_height / 2)
	r2 = T.move_y(r2, top_height / 2 - beam_recess)
	return union(r1, r2)
end

local top = make_top()
local beams = make_beams()
local attachment = make_attach()
local end_rails = make_end_rails()
local side_rails = make_side_rails()

local shape = union(top, beams, side_rails, end_rails)

emit(shape)
