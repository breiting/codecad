-- Rod bracket: rectangle + top semicircle ("D"-profile) with a through-hole; then extrude
-- Params:
--   w        : overall width of the bracket (X)
--   h_rect   : rectangle height (Y)
--   r_top    : radius of the top semicircle (defaults to w/2)
--   thick    : extrusion thickness (Z)
--   hole_d   : hole diameter (through Z)
--   seg      : segment count for arc/circle approximation

local mech = require("mech.rod")
local T = require("util.transform")
local frame = require("util.frame")

local w = param("w", 30)
local h_rect = param("h_rect", 20)
local r_top = param("r_top", w * 0.5)
local thick = param("thick", 6)
local hole_d = param("hole_d", 5)
local rod_length = param("rod_length", 100)

local function make_holder()
	return T.move_x(T.rot_x(T.rot_y(mech.rod_holder(w, h_rect, r_top, thick, hole_d), -90), 90), thick)
end

local holder_left = make_holder()
local holder_right = T.move_x(make_holder(), rod_length - thick)

frame.with_origin(0, -w / 2, h_rect, function()
	local rod = frame.apply(mech.rod(hole_d, rod_length))
	save_stl(union(holder_left, holder_right, rod), "out/rod_bracket.stl")
end)
