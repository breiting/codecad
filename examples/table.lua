-- Create a simple rod
local function rod(outer_d, thickness, length)
	local outer = cylinder(outer_d / 2, length)
	local inner = cylinder(outer_d / 2 - thickness, length)
	return union(difference(outer, inner), translate(box(40, 40, 40), -20, -20, length))
end

local function place(shape, x, y, z)
	local t = shape
	return translate(t, x or 0, y or 0, z or 0)
end

local r1 = rod(50, 2, 800)
local r2 = place(rod(50, 2, 800), 500)
local r3 = place(rod(50, 2, 800), 0, 500)

emit(union(r1, r2, r3))
