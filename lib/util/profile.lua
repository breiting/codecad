local M = {}

function M.rect(w, h)
	return poly_xy({ { 0, 0 }, { w, 0 }, { w, h }, { 0, h } }, true)
end

function M.ring(outer_d, inner_d)
	local o = cylinder(outer_d, 1)
	local i = translate(cylinder(inner_d, 1.2), 0, 0, -0.1)
	return difference(o, i)
end

--- Simple slot (rounded ends) in XY via difference helper
function M.slot_xy(w, h, t)
	local core = translate(box(w - t, h, 1.2), 0, 0, -0.1)
	local caps =
		union(translate(cylinder(t, 1.2), -(w - t) / 2, 0, -0.1), translate(cylinder(t, 1.2), (w - t) / 2, 0, -0.1))
	return union(core, caps)
end

-- ---- Arc/circle as point lists (degrees) ----
function M.arc_points(cx, cy, r, a0, a1, n)
	local pts = {}
	local da = (a1 - a0) / n
	for i = 0, n do
		local a = math.rad(a0 + i * da)
		pts[#pts + 1] = { cx + r * math.cos(a), cy + r * math.sin(a) }
	end
	return pts
end

local function append_pts(dst, src, start_idx)
	local eps = 1e-7
	local from = start_idx or 1
	for i = from, #src do
		local p = src[i]
		local q = dst[#dst]
		if not q or math.abs(p[1] - q[1]) > eps or math.abs(p[2] - q[2]) > eps then
			dst[#dst + 1] = p
		end
	end
end

-- ---- Outer D-shape profile (2D) ----
-- Coordinate system in XY: X across width, Y upwards.
-- Rectangle from (0,0) to (w, h_rect), then a semicircle centered at (w/2, h_rect).
function M.d_profile_points(w, h_rect, r_top, seg)
	-- start at bottom-left, go CCW up the left side
	local pts = {
		{ 0, 0 },
		{ 0, h_rect },
	}
	-- top semicircle center and samples
	local cx, cy = w * 0.5, h_rect
	local arc = M.arc_points(cx, cy, r_top, 180, 0, seg) -- includes both ends

	-- IMPORTANT: skip arc[1] because it's equal to {0, h_rect}
	append_pts(pts, arc, 2)

	-- then down the right edge to bottom-right
	pts[#pts + 1] = { w, 0 }
	-- closed=true will close back to {0,0}
	return pts
end

function M.circle_points(cx, cy, r, n)
	return M.arc_points(cx, cy, r, 0, 360, n)
end

return M
