--- @module "ccad.util.shape2d"
-- 2D shape builders for the XY-plane (return planar Faces).
-- Use with extrude()/revolve() to create solids.

local shape2d = {}

-- internal helpers ------------------------------------------------------------

local EPS = 1e-7

local function dedup_append(dst, src, start_idx)
	local from = start_idx or 1
	for i = from, #src do
		local p = src[i]
		local q = dst[#dst]
		if not q or math.abs(p[1] - q[1]) > EPS or math.abs(p[2] - q[2]) > EPS then
			dst[#dst + 1] = p
		end
	end
end

local function arc_points(cx, cy, r, a0_deg, a1_deg, n)
	local pts = {}
	local da = (a1_deg - a0_deg) / n
	for i = 0, n do
		local a = math.rad(a0_deg + i * da)
		pts[#pts + 1] = { cx + r * math.cos(a), cy + r * math.sin(a) }
	end
	return pts
end

-- primitives ------------------------------------------------------------------

--- Axis-aligned rectangle face with lower-left at (0,0).
--- @param w number width (X)
--- @param d number depth (Y)
--- @return Shape  -- planar face
function shape2d.rect(w, d)
	return poly_xy({ { 0, 0 }, { w, 0 }, { w, d }, { 0, d } }, true)
end

--- Circle face centered at (0,0) with diameter d.
--- @param d number diameter
--- @param seg? integer tessellation (default 64)
--- @return Shape
function shape2d.circle(d, seg)
	seg = seg or 64
	local r = d * 0.5
	local pts = {}
	local da = 360 / seg
	for i = 0, seg - 1 do
		local a = math.rad(i * da)
		pts[#pts + 1] = { r * math.cos(a), r * math.sin(a) }
	end
	return poly_xy(pts, true)
end

--- Annulus (ring) as a single face with an inner hole.
--- Outer center at (0,0), diameter do; inner diameter di.
--- @param d_o number outer diameter
--- @param d_i number inner diameter (must be < do)
--- @param seg? integer tessellation (default 96)
--- @return Shape
function shape2d.ring(d_o, d_i, seg)
	seg = seg or 96
	local ro = d_o * 0.5
	local ri = d_i * 0.5
	assert(ri > 0 and ri < ro, "ring: inner diameter must be >0 and < outer diameter")

	-- Outer CCW, inner CW (hole) for robust face orientation.
	local outer = {}
	local inner = {}
	for i = 0, seg - 1 do
		local a = math.rad(i * (360 / seg))
		outer[#outer + 1] = { ro * math.cos(a), ro * math.sin(a) }
	end
	for i = seg - 1, 0, -1 do
		local a = math.rad(i * (360 / seg))
		inner[#inner + 1] = { ri * math.cos(a), ri * math.sin(a) }
	end

	-- poly_xy supports multiple loops as nested arrays: { outer, hole1, hole2, ... }
	return poly_xy({ outer, inner }, true)
end

--- Rounded slot (obround) face aligned along X.
--- Overall length w, height h (along Y), end radius = h/2, straight mid section length = w - h.
--- @param w number overall length
--- @param h number overall height
--- @param seg? integer arc tessellation per 180° (default 32)
--- @return Shape
function shape2d.slot_xy(w, h, seg)
	seg = seg or 32
	local r = h * 0.5
	assert(w >= h, "slot_xy: length w must be >= height h")

	local half = (w - h) * 0.5
	-- Build polygon CCW: start at left mid, go up, left arc, down, straight, right arc, back to start.
	local pts = {}
	-- left straight up
	dedup_append(pts, { { -half, -r }, { -half, r } })
	-- left semicircle CCW around (-half, 0)
	local left_arc = arc_points(-half, 0, r, 90, 270, seg) -- from top to bottom (CCW)
	dedup_append(pts, left_arc, 2)
	-- bottom straight
	dedup_append(pts, { { -half, -r }, { half, -r } }, 2)
	-- right semicircle CCW around (+half, 0)
	local right_arc = arc_points(half, 0, r, 270, 90, seg) -- from bottom to top (CCW)
	dedup_append(pts, right_arc, 2)
	-- top straight back to start
	dedup_append(pts, { { half, r }, { -half, r } }, 2)

	return poly_xy(pts, true)
end

--- “D-profile” outline: rectangle (0..w, 0..h_rect) plus top semicircle.
--- Returns a Face; useful for brackets, holders, etc.
--- @param w number width (X)
--- @param h_rect number rectangular height (Y)
--- @param r_top number top semicircle radius (should be ~ w/2)
--- @param seg? integer arc tessellation (default 48)
--- @return Shape
function shape2d.d_profile_face(w, h_rect, r_top, seg)
	seg = seg or 48
	local pts = {
		{ 0, 0 },
		{ 0, h_rect },
	}
	local cx, cy = 0.5 * w, h_rect
	local arc = arc_points(cx, cy, r_top, 180, 0, seg) -- includes both ends
	dedup_append(pts, arc, 2)
	pts[#pts + 1] = { w, 0 }                        -- right-down
	return poly_xy(pts, true)
end

return shape2d
