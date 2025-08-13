-- Print-in-place hinged box
--
local box = require("util.box")
local T = require("util.transform")
local mech = require("mech.rod")

-- ===== Params =====
local L = param("L", 80)      -- length  (X)
local W = param("W", 40)      -- width   (Y)
local H = param("H", 20)      -- height  (Z)
local t = param("wall", 1)    -- wall thickness
local pin = param("pin", 2)   -- pin size
local edge = param("edge", 0) -- small chamfer
local hinge_height = H / 2

local function make_hinge_holder(w)
	local h_rect = 5
	local r_top = param("r_top", w * 0.5)
	local thick = t * 5
	local hole_d = param("hole_d", 5)
	local rod_length = L

	local function make_holder()
		return T.move_z(
			T.move_x(T.rot_x(T.rot_y(mech.rod_holder(w, h_rect, r_top, thick, hole_d), -90), 180), thick),
			w
		)
	end

	local holder_left = make_holder()
	local holder_right = T.move_x(make_holder(), rod_length - thick)

	local rod = translate(mech.rod(hole_d, rod_length), 0, -h_rect, r_top)
	return union(holder_left, holder_right, rod)
end

-- Open box
local bottom = box.open_box(L, W, H, t, edge)
local hinge_holder = T.move_z(make_hinge_holder(hinge_height), hinge_height)

local res = union(bottom, hinge_holder)

emit(res)
-- save_stl(bottom_half, "out/bottom_half_with_hinge.stl")
