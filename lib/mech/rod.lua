local profile = require("util.profile")

local M = {}

local rod_holder_segments = 24 -- segments for rounden rod_holder

--- Horizontal cylinder
function M.rod(diameter, length)
	return rotate_y(cylinder(diameter, length), 90)
end

--- Top-rounden rod holder with a optional hole in the middle (if diameter is missing or 0, no hole)
--- @param width number @total width in mm
--- @param height_bottom number @height of the rectangular shape
--- @param height_top number @height of the curved head (radius)
--- @param thickness number @thickness of extrusion
--- @param diameter number @optional if a rod hole should be applied
function M.rod_holder(width, height_bottom, height_top, thickness, diameter)
	local outer_face = poly_xy(profile.d_profile_points(width, height_bottom, height_top, rod_holder_segments), true)

	-- ---- Solid by extrusion (along Z) ----
	local solid = extrude(outer_face, thickness)

	if diameter and diameter > 0 then
		-- ---- Through hole (Z) ----
		local hx, hy = width * 0.5, height_bottom
		local hole = translate(cylinder(diameter, thickness + 0.2), hx, hy, -0.1)
		return difference(solid, hole)
	end
	return solid
end

return M
