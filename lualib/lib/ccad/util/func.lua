--- @module "ccad.util.func"
-- Sampling utilities to generate polylines and faces from mathematical functions.
-- Focus: deterministic point generation for cartesian/parametric/polar curves,
-- plus helpers to build planar faces or revolve profiles.
-- Units: mm. Angles: degrees (for polar APIs), radians where noted.

local F = {}

-- ============================================================================
-- internal helpers
-- ============================================================================

local EPS = 1e-7

local function is_finite(x)
	return type(x) == "number" and x == x and x ~= math.huge and x ~= -math.huge
end

--- Remove exact/near duplicates between consecutive points.
---@param pts number[][]
---@param eps number|nil
---@return number[][]
local function dedupe(pts, eps)
	eps = eps or EPS
	local n = #pts
	if n <= 1 then
		return pts
	end
	local out = { pts[1] }
	for i = 2, n do
		local a, b = out[#out], pts[i]
		if math.abs(a[1] - b[1]) > eps or math.abs(a[2] - b[2]) > eps then
			out[#out + 1] = b
		end
	end
	return out
end

--- Append `src` to `dst` skipping a possibly identical boundary point.
---@param dst number[][]
---@param src number[][]
---@param start_idx integer|nil
---@param eps number|nil
local function append_pts(dst, src, start_idx, eps)
	eps = eps or EPS
	local from = start_idx or 1
	for i = from, #src do
		local p = src[i]
		local q = dst[#dst]
		if not q or math.abs(p[1] - q[1]) > eps or math.abs(p[2] - q[2]) > eps then
			dst[#dst + 1] = p
		end
	end
end

-- ============================================================================
-- 1) Cartesian sampling: y = f(x)
-- ============================================================================

--- Sample y = f(x) over [x0, x1] with `n` points (>=2).
--- Invalid samples (NaN/Inf) are skipped by default.
---@param f fun(x:number):number
---@param x0 number
---@param x1 number
---@param n integer|nil  number of samples (default 100)
---@param keep_invalid? boolean|nil  if true, keep even invalids (not recommended)
---@return number[][]  array of {x,y}
function F.sample_xy(f, x0, x1, n, keep_invalid)
	n = math.max(2, math.floor(n or 100))
	local pts, dx = {}, (x1 - x0) / (n - 1)
	for i = 0, n - 1 do
		local x = x0 + i * dx
		local y = f(x)
		if is_finite(x) and is_finite(y) then
			pts[#pts + 1] = { x, y }
		elseif keep_invalid then
			pts[#pts + 1] = { x, y }
		end
	end
	return dedupe(pts)
end

-- ============================================================================
-- 2) Parametric sampling: x = fx(t), y = fy(t)
-- ============================================================================

--- Sample a parametric curve (x(t), y(t)) over [t0, t1] with `n` points (>=2).
---@param fx fun(t:number):number
---@param fy fun(t:number):number
---@param t0 number
---@param t1 number
---@param n integer|nil  default 200
---@return number[][]
function F.sample_param(fx, fy, t0, t1, n)
	n = math.max(2, math.floor(n or 200))
	local pts, dt = {}, (t1 - t0) / (n - 1)
	for i = 0, n - 1 do
		local t = t0 + i * dt
		local x, y = fx(t), fy(t)
		if is_finite(x) and is_finite(y) then
			pts[#pts + 1] = { x, y }
		end
	end
	return dedupe(pts)
end

-- ============================================================================
-- 3) Polar sampling: r = fr(theta) (theta in DEGREES for API convenience)
-- ============================================================================

--- Sample a polar curve r = fr(theta_rad) with theta in **degrees** [a0..a1].
--- Your function `fr` receives **radians** (theta_rad) to ease math usage.
---@param fr fun(theta_rad:number):number  returns radius r
---@param a0_deg number
---@param a1_deg number
---@param n integer|nil  default 200
---@return number[][]
function F.sample_polar(fr, a0_deg, a1_deg, n)
	n = math.max(2, math.floor(n or 200))
	local pts, da = {}, (a1_deg - a0_deg) / (n - 1)
	for i = 0, n - 1 do
		local a_deg = a0_deg + i * da
		local r = fr(math.rad(a_deg))
		if is_finite(r) then
			local a = math.rad(a_deg)
			local x = r * math.cos(a)
			local y = r * math.sin(a)
			if is_finite(x) and is_finite(y) then
				pts[#pts + 1] = { x, y }
			end
		end
	end
	return dedupe(pts)
end

-- ============================================================================
-- 4) Geometric helpers
-- ============================================================================

--- Uniformly sample a circular arc from a chord length.
--- Given radius R and chord length L, produce N points along the arc.
--- Arc is centered so that the chord runs horizontally; returns local arc coords.
---@param R number radius
---@param L number chord length
---@param N integer number of points (>=2)
---@return number[][]
function F.sample_arc(R, L, N)
	assert(R > 0, "sample_arc: R must be > 0")
	assert(L > 0 and L < 2 * R, "sample_arc: L must be > 0 and < 2*R")
	N = math.max(2, math.floor(N or 32))

	local pts = {}
	local a = L * 0.5 -- half chord
	local h = math.sqrt(R * R - a * a)
	local theta = 2 * math.asin(a / R)

	for i = 0, N - 1 do
		local phi = -theta * 0.5 + i * (theta / (N - 1))
		local x = R * math.cos(phi) - h
		local y = R * math.sin(phi)
		pts[#pts + 1] = { x, y }
	end
	return pts
end

-- ============================================================================
-- 5) Build planar faces from point sets
-- ============================================================================

--- Build an XY planar face from a polyline; closed by default.
---@param points number[][] array of {x,y}
---@param closed? boolean
---@return Shape
function F.face_xy(points, closed)
	return poly_xy(dedupe(points), closed ~= false)
end

--- Rectangle whose **top** edge follows y=f(x) over [xL, xR].
--- Lower edge is a straight segment at yBottom; the left join is yJoin.
---@param xL number
---@param xR number
---@param yBottom number
---@param yJoin number
---@param f fun(x:number):number
---@param n integer|nil sample count for top (default 100)
---@return Shape
function F.rect_with_func_top(xL, xR, yBottom, yJoin, f, n)
	local pts = { { xL, yBottom }, { xL, yJoin } }
	local top = F.sample_xy(f, xL, xR, n or 100)
	-- Skip first top point if identical to {xL, yJoin}
	append_pts(pts, top, 2)
	pts[#pts + 1] = { xR, yBottom }
	return F.face_xy(pts, true)
end

-- ============================================================================
-- 6) 3D convenience (explicit)
-- ============================================================================

--- Convenience: revolve a polar-defined radius r(theta) into a 360° solid.
--- Interprets polar samples as (x,y)→(r,z) in XZ and revolves around Z.
---@param fr fun(theta_rad:number):number
---@param a0_deg number
---@param a1_deg number
---@return Shape
function F.revolve_from_polar(fr, a0_deg, a1_deg)
	local pol = F.sample_polar(fr, a0_deg, a1_deg, 256)
	local rz = {}
	for i = 1, #pol do
		local x, y = pol[i][1], pol[i][2]
		local r = math.sqrt(x * x + y * y)
		local z = y
		rz[#rz + 1] = { r, z }
	end
	local profile = poly_xz(rz, false, true)
	return revolve(profile, 360)
end

return F
