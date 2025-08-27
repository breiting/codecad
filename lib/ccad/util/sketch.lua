--- @module "ccad.util.sketch"
-- Tiny, chainable 2D sketch DSL for XY profiles (lines and arcs).
-- Produces a single closed or open polyline and turns it into a planar face via poly_xy().
-- Units: mm, Angles: degrees (CCW positive).

local S = {}
S.__index = S

-- --- internal helpers -------------------------------------------------------

local EPS = 1e-9

local function almost_equal(a, b)
	return math.abs(a - b) <= EPS
end

local function add_point(self, x, y)
	local pts = self.pts
	local last = pts[#pts]
	if not last or not (almost_equal(last[1], x) and almost_equal(last[2], y)) then
		pts[#pts + 1] = { x, y }
	end
end

local function clamp_int(v, lo, hi)
	v = math.floor(v or lo)
	if v < lo then
		return lo
	end
	if v > hi then
		return hi
	end
	return v
end

-- --- constructor ------------------------------------------------------------

--- Begin a sketch at (x, y).
--- If omitted, starts at the origin (0,0).
--- @param x? number
--- @param y? number
--- @return ccad.util.sketch.Sketch
function S.begin(x, y)
	local self = setmetatable({
		pts = { { x or 0, y or 0 } },
		closed = false,
	}, S)
	return self
end

-- --- path building ----------------------------------------------------------

--- Move to absolute (x, y), starting a fresh path (single loop model).
--- @param x number
--- @param y number
--- @return ccad.util.sketch.Sketch
function S:move_to(x, y)
	self.pts = { { x, y } }
	self.closed = false
	return self
end

--- Line to absolute (x, y).
--- Adds the point only if it differs from the current end point.
--- @param x number
--- @param y number
--- @return ccad.util.sketch.Sketch
function S:line_to(x, y)
	add_point(self, x, y)
	return self
end

--- Circular arc **clockwise** around center (cx, cy) with radius r,
--- starting from current point, sweeping by `sweep_deg`.
--- @param cx number @arc center X
--- @param cy number @arc center Y
--- @param r number  @radius (mm), must be > 0
--- @param sweep_deg number @CW sweep angle in degrees (>0 recommended)
--- @param seg? integer @tessellation segments (per whole sweep), default 16, min 4
--- @return ccad.util.sketch.Sketch
function S:arc_cw(cx, cy, r, sweep_deg, seg)
	r = math.max(r or 0, 0)
	if r <= 0 then
		return self
	end
	seg = clamp_int(seg or 16, 4, 4096)

	local p = self.pts[#self.pts]
	local a0 = math.atan(p[2] - cy, p[1] - cx) -- atan2(y, x)
	local a1 = a0 - math.rad(sweep_deg)     -- CW: subtract
	local da = (a1 - a0) / seg

	for i = 1, seg do
		local a = a0 + i * da
		add_point(self, cx + r * math.cos(a), cy + r * math.sin(a))
	end
	return self
end

--- Circular arc **counter-clockwise** around center (cx, cy) with radius r,
--- starting from current point, sweeping by `sweep_deg`.
--- @param cx number @arc center X
--- @param cy number @arc center Y
--- @param r number  @radius (mm), must be > 0
--- @param sweep_deg number @CCW sweep angle in degrees (>0 recommended)
--- @param seg? integer @tessellation segments (per whole sweep), default 16, min 4
--- @return ccad.util.sketch.Sketch
function S:arc_ccw(cx, cy, r, sweep_deg, seg)
	r = math.max(r or 0, 0)
	if r <= 0 then
		return self
	end
	seg = clamp_int(seg or 16, 4, 4096)

	local p = self.pts[#self.pts]
	local a0 = math.atan(p[2] - cy, p[1] - cx) -- atan2(y, x)
	local a1 = a0 + math.rad(sweep_deg)     -- CCW: add
	local da = (a1 - a0) / seg

	for i = 1, seg do
		local a = a0 + i * da
		add_point(self, cx + r * math.cos(a), cy + r * math.sin(a))
	end
	return self
end

--- Close the path back to the first point.
--- @return ccad.util.sketch.Sketch
function S:close()
	self.closed = true
	return self
end

-- --- finalize ---------------------------------------------------------------

--- Build a planar face using the accumulated polyline.
--- Uses poly_xy(points, closed). If not explicitly closed, will close by default.
--- @return Shape
function S:face()
	return poly_xy(self.pts, self.closed ~= false)
end

return S
