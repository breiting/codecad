-- lib/util/place.lua
local M = {}

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

-- Erzeugt nx Stück entlang X (dx Abstand), optional ny entlang Y (dy Abstand)
function M.array(make_shape, nx, dx, ny, dy, z)
	local parts = {}
	for i = 0, (nx - 1) do
		local col = make_shape()
		col = translate(col, i * dx, 0, z or 0)
		parts[#parts + 1] = col
	end
	if ny and ny > 1 then
		local rows = {}
		for j = 0, (ny - 1) do
			local row = {}
			for i = 1, #parts do
				row[i] = translate(parts[i], 0, j * dy, 0)
			end
			rows[#rows + 1] = union(table.unpack(row))
		end
		return union(table.unpack(rows))
	end
	return union(table.unpack(parts))
end

return M
