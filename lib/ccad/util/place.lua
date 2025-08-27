--- @module "ccad.util.place"
-- Placement helpers for replicating shapes in space.
-- Units: mm. Angles: degrees.

local M = {}

-- Internal: fold-union over a list, must not be empty
local function union_list(list)
	assert(#list > 0, "union_list called with empty list")
	local acc = list[1]
	for i = 2, #list do
		acc = union(acc, list[i])
	end
	return acc
end

----------------------------------------------------------------------
-- Single placement
----------------------------------------------------------------------

--- Place a shape with optional uniform scale and Euler rotations, then translate.
--- Missing values are treated as 0 (or 1 for scale).
--- Order: scale → rotX → rotY → rotZ → translate.
--- @param s Shape
--- @param x? number @translate X
--- @param y? number @translate Y
--- @param z? number @translate Z
--- @param rx? number @rotate X (deg)
--- @param ry? number @rotate Y (deg)
--- @param rz? number @rotate Z (deg)
--- @param sc? number @uniform scale
--- @return Shape
function M.place(s, x, y, z, rx, ry, rz, sc)
	local t = s
	if sc and sc ~= 1 then
		t = scale(t, sc)
	end
	if rx and rx ~= 0 then
		t = rotate_x(t, rx)
	end
	if ry and ry ~= 0 then
		t = rotate_y(t, ry)
	end
	if rz and rz ~= 0 then
		t = rotate_z(t, rz)
	end
	return translate(t, x or 0, y or 0, z or 0)
end

----------------------------------------------------------------------
-- Linear / grid replication
----------------------------------------------------------------------

--- Create a 1D array along X; optionally replicated along Y (grid).
--- The maker function is called as `make(i, j)` with 0-based indices.
--- If `ny` is nil or <=1, only one row along X is produced.
--- @param make fun(i:integer, j:integer): Shape
--- @param nx integer @number of items in X (>=1)
--- @param dx number  @spacing in X [mm]
--- @param ny? integer @rows in Y (>=1)
--- @param dy? number  @spacing in Y [mm]
--- @return Shape
function M.array(make, nx, dx, ny, dy)
	assert(nx and nx >= 1, "array: nx must be >= 1")
	assert(dx, "array: dx required")
	ny = ny or 1
	dy = dy or 0

	-- Build one row
	local row = {}
	for i = 0, nx - 1 do
		local s = make(i, 0)
		row[#row + 1] = translate(s, i * dx, 0, 0)
	end
	local row_union = union_list(row)

	if ny <= 1 then
		return row_union
	end

	-- Replicate rows
	local rows = {}
	for j = 0, ny - 1 do
		rows[#rows + 1] = translate(row_union, 0, j * dy, 0)
	end
	return union_list(rows)
end

--- 2D grid (X/Y) using a maker function `(i, j) -> Shape`.
--- Convenience wrapper around `array`.
--- @param make fun(i:integer, j:integer): Shape
--- @param nx integer
--- @param dx number
--- @param ny integer
--- @param dy number
--- @return Shape
function M.grid(make, nx, dx, ny, dy)
	return M.array(make, nx, dx, ny, dy)
end

----------------------------------------------------------------------
-- Polar placement
----------------------------------------------------------------------

--- Place `n` instances around a circle of radius `r` in the XY-plane.
--- Angle 0° points along +X; increases CCW.
--- The maker function is called as `make(i, angle_deg)` with i in [0..n-1].
--- @param n integer  @number of items (>=1)
--- @param r number   @radius [mm]
--- @param make fun(i:integer, angle_deg:number): Shape
--- @param angle0? number @start angle (deg), default 0
--- @return Shape
function M.polar(n, r, make, angle0)
	assert(n and n >= 1, "polar: n must be >= 1")
	assert(r and r >= 0, "polar: r must be >= 0")
	angle0 = angle0 or 0

	local parts = {}
	for i = 0, n - 1 do
		local a = angle0 + i * (360 / n)
		local rad = math.rad(a)
		local x = r * math.cos(rad)
		local y = r * math.sin(rad)
		local s = make(i, a)
		parts[#parts + 1] = translate(s, x, y, 0)
	end
	return union_list(parts)
end

return M
