--- @module "ccad.util.transform"
-- Thin convenience layer around CodeCAD core transforms.
-- Units: mm. Angles: degrees (right-hand rule). Coordinate system: Z up.

local M = {}

----------------------------------------------------------------------
-- Translation helpers
----------------------------------------------------------------------

--- Translate a shape along X by dx.
--- @param shape Shape
--- @param dx number distance in mm
--- @return Shape
function M.move_x(shape, dx)
	return translate(shape, dx, 0, 0)
end

--- Translate a shape along Y by dy.
--- @param shape Shape
--- @param dy number distance in mm
--- @return Shape
function M.move_y(shape, dy)
	return translate(shape, 0, dy, 0)
end

--- Translate a shape along Z by dz.
--- @param shape Shape
--- @param dz number distance in mm
--- @return Shape
function M.move_z(shape, dz)
	return translate(shape, 0, 0, dz)
end

--- Translate a shape by (dx, dy, dz).
--- @param shape Shape
--- @param dx number
--- @param dy number
--- @param dz number
--- @return Shape
function M.move(shape, dx, dy, dz)
	return translate(shape, dx, dy, dz)
end

----------------------------------------------------------------------
-- Rotation helpers
-- (rot_* rotate around world axes through the origin)
----------------------------------------------------------------------

--- Rotate around world X-axis by angle (deg).
--- @param shape Shape
--- @param angle number degrees
--- @return Shape
function M.rot_x(shape, angle)
	return rotate_x(shape, angle)
end

--- Rotate around world Y-axis by angle (deg).
--- @param shape Shape
--- @param angle number degrees
--- @return Shape
function M.rot_y(shape, angle)
	return rotate_y(shape, angle)
end

--- Rotate around world Z-axis by angle (deg).
--- @param shape Shape
--- @param angle number degrees
--- @return Shape
function M.rot_z(shape, angle)
	return rotate_z(shape, angle)
end

--- Rotate with Euler angles in a specified order.
--- Default order is "xyz" (apply X, then Y, then Z).
--- Valid order strings: any permutation of "x", "y", "z" (e.g. "zyx").
--- @param shape Shape
--- @param rx number rotation about X (deg)
--- @param ry number rotation about Y (deg)
--- @param rz number rotation about Z (deg)
--- @param order? '"xyz"'|'"xzy"'|'"yxz"'|'"yzx"'|'"zxy"'|'"zyx"'
--- @return Shape
function M.rot_euler(shape, rx, ry, rz, order)
	order = order or "xyz"
	local s = shape
	for i = 1, #order do
		local c = order:sub(i, i)
		if c == "x" then
			s = rotate_x(s, rx)
		elseif c == "y" then
			s = rotate_y(s, ry)
		elseif c == "z" then
			s = rotate_z(s, rz)
		else
			error("rot_euler: invalid order character '" .. c .. "'")
		end
	end
	return s
end

----------------------------------------------------------------------
-- Centering (namespaced re-exports for convenience)
-- (These call the core globals so users can write util.transform.center_xy(...))
----------------------------------------------------------------------

--- Center on X (translates by half width).
--- @param shape Shape
--- @return Shape
function M.center_x(shape)
	return center_x(shape)
end

--- Center on Y.
--- @param shape Shape
--- @return Shape
function M.center_y(shape)
	return center_y(shape)
end

--- Center on Z.
--- @param shape Shape
--- @return Shape
function M.center_z(shape)
	return center_z(shape)
end

--- Center on X and Y.
--- @param shape Shape
--- @return Shape
function M.center_xy(shape)
	return center_xy(shape)
end

--- Center on X, Y and Z.
--- @param shape Shape
--- @return Shape
function M.center_xyz(shape)
	return center_xyz(shape)
end

--- Translate so that the bounding-box center lands at (cx,cy,cz).
--- @param shape Shape
--- @param cx number
--- @param cy number
--- @param cz number
--- @return Shape
function M.center_to(shape, cx, cy, cz)
	return center_to(shape, cx, cy, cz)
end

return M
