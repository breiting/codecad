--- @module "util.box"
--- Rounded rectangle (2D) + extrusions (3D)
--- Build arcs + lines in 2D, then extrude along Z.
local sketch = require("util.sketch")

local M = {}

--- Generates an open box with a certain size and wall thickness
--- @param L number @length (mm)
--- @param W number @width (mm)
--- @param H number @height (mm)
--- @param t number @wall thickness (mm)
function M.open(L, W, H, t, edge)
	local outer = box(L, W, H)
	local inner = translate(box(L - 2 * t, W - 2 * t, H - t), t, t, t)
	local r = difference(outer, inner)
	if edge and edge > 0 then
		return fillet(r, edge)
	end
	return r
end

function M.centered(w, d, h)
	return center_xy(box(w, d, h))
end

-- Clamp helper
local function clamp(v, lo, hi)
	return math.max(lo, math.min(hi, v))
end

--- Rounded rectangle as Face in XY (origin at bottom-left).
--- @param w number  width (X)
--- @param d number  depth (Y)
--- @param r number  corner radius (>=0)
--- @param seg? integer  arc segments per quarter (default 16)
--- @return Shape  (Face)
function M.rounded2d(w, d, r, seg)
	seg = seg or 16
	r = math.max(0, r or 0)
	local rmax = 0.5 * math.min(w, d) - 1e-6
	r = clamp(r, 0, rmax)

	if r == 0 then
		return poly_xy({ { 0, 0 }, { w, 0 }, { w, d }, { 0, d } }, true)
	end

	-- Path: start at (r,0), go CCW, arcs are 90° at each corner
	local s = sketch
		.begin(r, 0)
		:line_to(w - r, 0)
		:arc_ccw(w - r, r, r, 90, seg) -- lower-right corner
		:line_to(w, d - r)
		:arc_ccw(w - r, d - r, r, 90, seg) -- upper-right
		:line_to(r, d)
		:arc_ccw(r, d - r, r, 90, seg) -- upper-left
		:line_to(0, r)
		:arc_ccw(r, r, r, 90, seg)   -- lower-left back to start
	-- :close()

	return s:face()
end

--- Rounded solid by extruding the rounded2d profile.
--- @param w number
--- @param d number
--- @param h number
--- @param r number
--- @param seg? integer
--- @return Shape  Solid
function M.rounded(w, d, h, r, seg)
	local face = M.rounded2d(w, d, r, seg)
	return extrude(face, h)
end

--- Hollow rounded box (open or closed top).
--- Outer rounded box minus inner rounded box with reduced size + radius.
--- @param w number  outer width
--- @param d number  outer depth
--- @param h number  height
--- @param t number  wall thickness (uniform)
--- @param r number  outer corner radius
--- @param seg? integer
--- @param open_top? boolean  if true, inner height = h (offen nach oben)
--- @return Shape
function M.rounded_shell(w, d, h, t, r, seg, open_top)
	t = math.max(0, t)
	seg = seg or 16

	local outer = M.rounded(w, d, h, r, seg)

	local wi, di = w - 2 * t, d - 2 * t
	if wi <= 0 or di <= 0 then
		-- wall too big → return solid outer
		return outer
	end

	local ri = math.max(0, r - t)     -- inner radius reduced by wall
	local inner_face = M.rounded2d(wi, di, ri, seg)
	local hi = open_top and h or (h - t) -- offene oder geschlossene Variante
	if hi <= 0 then
		return outer
	end
	local inner = translate(extrude(inner_face, hi), t, t, open_top and 0 or t)

	return difference(outer, inner)
end

return M
