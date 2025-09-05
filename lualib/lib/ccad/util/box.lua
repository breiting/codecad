--- @module "ccad.util.box"
-- Convenience helpers to build rectangular and rounded boxes
-- using CodeCAD core primitives. Units: mm. Angles: degrees.

local sketch = require("ccad.util.sketch")

local M = {}

-- Internal clamp helper
local function clamp(v, lo, hi)
	return math.max(lo, math.min(hi, v))
end

----------------------------------------------------------------------
-- Rectangular thin-walled box (open at the top)
----------------------------------------------------------------------

--- Create a thin-walled rectangular box (open top).
--- Side walls have thickness `t`, bottom thickness is also `t`.
--- The outer solid starts at the origin; use center_* / translate as needed.
--- @param w number  outer width  (X)
--- @param d number  outer depth  (Y)
--- @param h number  height       (Z)
--- @param t number  wall thickness (uniform)
--- @param fillet_radius? number  optional global edge fillet radius
--- @return Shape
function M.open_box(w, d, h, t, fillet_radius)
	local outer = box(w, d, h)
	local iw, id, ih = w - 2 * t, d - 2 * t, h - t
	if iw <= 0 or id <= 0 or ih <= 0 then
		-- Walls too thick → fall back to solid outer
		return outer
	end
	local inner = translate(box(iw, id, ih), t, t, t)
	local r = difference(outer, inner)
	if fillet_radius and fillet_radius > 0 then
		return fillet(r, fillet_radius)
	end
	return r
end

--- Convenience: centered rectangular solid (XY centered around 0).
--- @param w number
--- @param d number
--- @param h number
--- @return Shape
function M.centered(w, d, h)
	return center_xy(box(w, d, h))
end

----------------------------------------------------------------------
-- Rounded rectangles (2D) and solids (3D)
----------------------------------------------------------------------

--- Build a rounded rectangle as a planar face in XY.
--- Origin is at the lower-left corner of the unrounded rectangle.
--- @param w number  width  (X)
--- @param d number  depth  (Y)
--- @param r number  corner radius (>= 0). Clamped to fit.
--- @param seg? integer  arc tessellation per 90° corner (default 16)
--- @return Shape  planar face
function M.rounded_rect_face(w, d, r, seg)
	seg = seg or 16
	r = math.max(0, r or 0)
	local rmax = 0.5 * math.min(w, d) - 1e-6
	r = clamp(r, 0, rmax)

	if r == 0 then
		return poly_xy({ { 0, 0 }, { w, 0 }, { w, d }, { 0, d } }, true)
	end

	-- Path: start at (r,0); CCW around the rectangle using four quarter arcs.
	local s = sketch
		.begin(r, 0)
		:line_to(w - r, 0)
		:arc_ccw(w - r, r, r, 90, seg) -- lower-right
		:line_to(w, d - r)
		:arc_ccw(w - r, d - r, r, 90, seg) -- upper-right
		:line_to(r, d)
		:arc_ccw(r, d - r, r, 90, seg) -- upper-left
		:line_to(0, r)
		:arc_ccw(r, r, r, 90, seg)   -- lower-left
	return s:face()
end

--- Extrude a rounded rectangle face to a 3D solid.
--- @param w number
--- @param d number
--- @param h number
--- @param r number
--- @param seg? integer
--- @return Shape
function M.rounded_box(w, d, h, r, seg)
	local face = M.rounded_rect_face(w, d, r, seg)
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
--- @param open_top? boolean  if true, inner height = h (open at the top)
--- @return Shape
function M.rounded_shell(w, d, h, t, r, seg, open_top)
	t = math.max(0, t)
	seg = seg or 16

	local outer = M.rounded_box(w, d, h, r, seg)

	local wi, di = w - 2 * t, d - 2 * t
	if wi <= 0 or di <= 0 then
		return outer -- walls too big
	end

	local ri = math.max(0, r - t) -- inner radius reduced by wall
	local inner_face = M.rounded_rect_face(wi, di, ri, seg)
	local hi = open_top and h or (h - t)
	if hi <= 0 then
		return outer
	end

	local inner = translate(extrude(inner_face, hi), t, t, open_top and 0 or t)
	return difference(outer, inner)
end

return M
