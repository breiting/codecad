local M = {}

function M.move_x(shape, dx)
	return translate(shape, dx, 0, 0)
end

function M.move_y(shape, dy)
	return translate(shape, 0, dy, 0)
end

function M.move_z(shape, dz)
	return translate(shape, 0, 0, dz)
end

function M.rot_x(shape, angle)
	return rotate_x(shape, angle)
end

function M.rot_y(shape, angle)
	return rotate_y(shape, angle)
end

function M.rot_z(shape, angle)
	return rotate_z(shape, angle)
end

return M
