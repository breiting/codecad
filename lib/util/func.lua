-- util/func.lua
-- Generate polylines and faces from math functions (cartesian, parametric, polar).
-- Robust against NaN/Inf and zero-length edges.

local F = {}

local function is_num(x)
	return type(x) == "number" and x == x and x ~= math.huge and x ~= -math.huge
end

local function dedupe_local(pts, eps)
	eps = eps or 1e-7
	if #pts <= 1 then
		return pts
	end
	local out = { pts[1] }
	for i = 2, #pts do
		local a, b = out[#out], pts[i]
		if math.abs(a[1] - b[1]) > eps or math.abs(a[2] - b[2]) > eps then
			out[#out + 1] = b
		end
	end
	return out
end

-- NEW: append src to dst, skipping duplicate boundary point
local function append_pts(dst, src, start_idx, eps)
	eps = eps or 1e-7
	local from = start_idx or 1
	for i = from, #src do
		local p = src[i]
		local q = dst[#dst]
		if not q or math.abs(p[1] - q[1]) > eps or math.abs(p[2] - q[2]) > eps then
			dst[#dst + 1] = p
		end
	end
end

-- 1) Cartesian: y = f(x)
function F.sample_xy(f, x0, x1, n, filter)
	n = math.max(2, math.floor(n or 100))
	local pts, dx = {}, (x1 - x0) / (n - 1)
	for i = 0, n - 1 do
		local x = x0 + i * dx
		local y = f(x)
		if is_num(x) and is_num(y) then
			pts[#pts + 1] = { x, y }
		elseif filter ~= false then
			-- skip invalid sample
		end
	end
	return dedupe_local(pts)
end

-- 2) Parametric: x = fx(t), y = fy(t)
function F.sample_param(fx, fy, t0, t1, n)
	n = math.max(2, math.floor(n or 200))
	local pts, dt = {}, (t1 - t0) / (n - 1)
	for i = 0, n - 1 do
		local t = t0 + i * dt
		local x, y = fx(t), fy(t)
		if is_num(x) and is_num(y) then
			pts[#pts + 1] = { x, y }
		end
	end
	return dedupe_local(pts)
end

-- 3) Polar: r = fr(a)  (a in degrees)
function F.sample_polar(fr, a0, a1, n)
	n = math.max(2, math.floor(n or 200))
	local pts, da = {}, (a1 - a0) / (n - 1)
	for i = 0, n - 1 do
		local a = a0 + i * da
		local r = fr(math.rad(a))
		if is_num(r) then
			local x = r * math.cos(math.rad(a))
			local y = r * math.sin(math.rad(a))
			if is_num(x) and is_num(y) then
				pts[#pts + 1] = { x, y }
			end
		end
	end
	return dedupe_local(pts)
end

function F.face_xy(points, closed)
	-- defensiv: lokale Dedupe noch mal laufen lassen
	return poly_xy(dedupe_local(points), closed ~= false)
end

-- 4) Convenience

-- Rectangle with a top edge defined by y=f(x) over [xL, xR]
function F.rect_with_func_top(xL, xR, yBottom, yJoin, f, n)
	local pts = { { xL, yBottom }, { xL, yJoin } }
	local top = F.sample_xy(f, xL, xR, n or 100)

	-- WICHTIG: ersten Top-Punkt überspringen, wenn er (xL, yJoin) entspricht
	append_pts(pts, top, 2) -- start at index 2 to avoid duplicate boundary point

	-- rechte Kante runter
	pts[#pts + 1] = { xR, yBottom }

	return F.face_xy(pts, true)
end

-- Revolve from polar radius r(theta) (optional utility)
function F.revolve_from_polar(fr, a0_deg, a1_deg, thickness_z)
	local pol = F.sample_polar(fr, a0_deg, a1_deg, 256)
	local rz = {}
	for i = 1, #pol do
		local r = math.sqrt(pol[i][1] ^ 2 + pol[i][2] ^ 2)
		local z = pol[i][2]
		rz[#rz + 1] = { r, z }
	end
	local prof = poly_xz(rz, false, true)
	return revolve(prof, 360)
end

return F
