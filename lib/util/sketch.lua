--- @module "util.sketch"
--- Tiny 2D sketch DSL for XY profiles (lines and arcs)
local S = {}
S.__index = S

local function new(x, y)
	return setmetatable({ pts = { { x, y } }, closed = false }, S)
end

--- Begin a sketch at (x,y)
function S.begin(x, y)
	return new(x or 0, y or 0)
end

--- Move to absolute (x,y) (starts new subpath); keep simple: one loop only.
function S:move_to(x, y)
	self.pts = { { x, y } }
	return self
end

--- Line to absolute (x,y)
function S:line_to(x, y)
	local p = self.pts[#self.pts]
	if p[1] ~= x or p[2] ~= y then
		self.pts[#self.pts + 1] = { x, y }
	end
	return self
end

--- Arc CCW (counter-clockwise) around center (cx,cy) from current point to angle sweep_deg.
--  If you prefer end-angle, change signature; for beginners: sweep is easier.
function S:arc_cw(cx, cy, r, sweep_deg, seg)
	seg = math.max(4, seg or 16)
	local p = self.pts[#self.pts]
	local a0 = math.atan(p[2] - cy, p[1] - cx)
	local a1 = a0 - math.rad(sweep_deg)
	local da = (a1 - a0) / seg
	for i = 1, seg do
		local a = a0 + i * da
		self.pts[#self.pts + 1] = { cx + r * math.cos(a), cy + r * math.sin(a) }
	end
	return self
end

function S:arc_ccw(cx, cy, r, sweep_deg, seg)
	seg = math.max(4, seg or 16)
	local p = self.pts[#self.pts]
	local a0 = math.atan(p[2] - cy, p[1] - cx)
	local a1 = a0 + math.rad(sweep_deg)
	local da = (a1 - a0) / seg
	for i = 1, seg do
		local a = a0 + i * da
		self.pts[#self.pts + 1] = { cx + r * math.cos(a), cy + r * math.sin(a) }
	end
	return self
end

--- Close shape back to the first point
function S:close()
	self.closed = true
	return self
end

--- Build a Face (poly_xy)
function S:face()
	return poly_xy(self.pts, self.closed ~= false)
end

return S
