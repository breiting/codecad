-- Simple adapter for the kitchen (real personal need)
--
local B = require("util.box")
local T = require("util.transform")

local base_len = 75
local base_w = 50
local base_h = 6
local corner_r = 5
local mount_offset = 27
local mount_d = 6.4
local shaft_height = 80
local shaft_od = 34
local shaft_wall = 8
local shaft_reinforcement = 10

local rib_w = 11
local rib_l = 20

local function holes()
	local hole_left = T.move_x(cylinder(mount_d, base_h), -mount_offset)
	local hole_right = T.move_x(cylinder(mount_d, base_h), mount_offset)
	local hole_center = cylinder(mount_d, base_h)
	return union(hole_left, hole_right, hole_center)
end

local function make_shaft_simple()
	local outer = cylinder(shaft_od, shaft_height)
	local inner = cylinder(shaft_od - shaft_wall, shaft_height)
	return difference(outer, inner)
end

local function make_shaft_enforced(height)
	local shift = height * (1 / 3)
	local p1 = cone(shaft_od, shaft_od + shaft_reinforcement, shift)
	local p2 = T.move_z(cylinder(shaft_od + shaft_reinforcement, shift), shift)
	local p3 = T.move_z(cone(shaft_od + shaft_reinforcement, shaft_od, shift), shift * 2)
	local merged = union(p1, p2, p3)
	local inner = T.move_z(cylinder(shaft_od - shaft_wall, height), base_h + shift)
	return difference(merged, inner)
end

-- simplified version
local function make_version1()
	local base = center_xy(B.rounded(base_len, base_w, base_h, corner_r))
	local shaft = make_shaft_simple()
	local rib1 = T.move_y(center_xy(box(rib_l, rib_w, shaft_height)), -base_w / 2 + rib_w / 2)
	local rib2 = T.move_y(center_xy(box(rib_l, rib_w, shaft_height)), base_w / 2 - rib_w / 2)

	base = difference(base, holes())

	return union(base, shaft, rib1, rib2)
end

-- improved version
local function make_version2()
	local base = center_xy(B.rounded(base_len, base_w, base_h, corner_r))
	local shaft = make_shaft_enforced(shaft_height - base_h)
	shaft = T.move_z(shaft, base_h)

	base = difference(base, holes())

	return union(base, shaft)
end

local function check(e, a)
	local diff = math.abs(e - a)
	return diff < 0.1
end

local _ = make_version1()
local shape = make_version2()

assert(check(shaft_height, bbox(shape).size.z), "Shaft height does not fit specification")

emit(shape)
