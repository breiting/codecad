-- lib/util/place.lua
local M = {}

-- Place a shape with optional scale and rotations
function M.place(s, x, y, z, rx, ry, rz, sc)
	local t = s
	if sc then
		t = scale(t, sc)
	end
	if rx then
		t = rotate_x(t, rx)
	end
	if ry then
		t = rotate_y(t, ry)
	end
	if rz then
		t = rotate_z(t, rz)
	end
	return translate(t, x or 0, y or 0, z or 0)
end

--- 1D array along X; optionally replicated along Y
function M.array(make_shape, nx, dx, ny, dy)
	local col = {}
	for i = 0, (nx - 1) do
		col[#col + 1] = translate(make_shape(), i * dx, 0, 0)
	end
	if ny and ny > 1 then
		local rows = {}
		for j = 0, (ny - 1) do
			local r = {}
			for i = 1, #col do
				r[i] = translate(col[i], 0, j * dy, 0)
			end
			rows[#rows + 1] = union(table.unpack(r))
		end
		return union(table.unpack(rows))
	end
	return union(table.unpack(col))
end

--- 2D grid (X/Y) of a given shape builder
function M.grid(make_shape, nx, dx, ny, dy)
	return M.array(make_shape, nx, dx, ny, dy)
end

return M
