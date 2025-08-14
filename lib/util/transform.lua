--- @module "util.transform"
--- Various helper functions
local M = {}

--- Moves the shape in x direction
--- @param shape Shape
--- @param dx number distance in mm
function M.move_x(shape, dx)
	return translate(shape, dx, 0, 0)
end

--- Moves the shape in y direction
--- @param shape Shape
--- @param dy number distance in mm
function M.move_y(shape, dy)
	return translate(shape, 0, dy, 0)
end

--- Moves the shape in z direction
--- @param shape Shape
--- @param dz number distance in mm
function M.move_z(shape, dz)
	return translate(shape, 0, 0, dz)
end

--- Rotates the shape around the x axis
--- @param shape Shape
--- @param angle number angle in degrees
function M.rot_x(shape, angle)
	return rotate_x(shape, angle)
end

--- Rotates the shape around the y axis
--- @param shape Shape
--- @param angle number angle in degrees
function M.rot_y(shape, angle)
	return rotate_y(shape, angle)
end

--- Rotates the shape around the z axis
--- @param shape Shape
--- @param angle number angle in degrees
function M.rot_z(shape, angle)
	return rotate_z(shape, angle)
end

return M
