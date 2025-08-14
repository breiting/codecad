-- examples/hose_adapter_sigmoid.lua
-- Universal hose adapter from radius r1 to r2 over height h,
-- smooth transition via normalized logistic (sigmoid). Revolved 360°.
-- Params:
--   r1      : lower radius  [mm]
--   r2      : upper radius  [mm]
--   h       : height        [mm]
--   wall    : wall thickness [mm]
--   k       : sigmoid steepness (typ. 6..16; higher = sharper middle)
--   n       : samples along height (resolution)
--   lip1    : optional straight lip length at bottom [mm]
--   lip2    : optional straight lip length at top [mm]

local r1 = param("r1", 20)
local r2 = param("r2", 25)
local h = param("h", 40)
local wall = param("wall", 1)
local k = param("k", 10) -- sigmoid steepness
local n = math.max(16, param("n", 200))
local lip1 = math.max(0, param("lip1", 10))
local lip2 = math.max(0, param("lip2", 10))

-- Guards
if h <= 0 then
	error("h must be > 0", 0)
end
if r1 <= 0 or r2 <= 0 then
	error("r1 and r2 must be > 0", 0)
end
if wall <= 0 then
	error("wall must be > 0", 0)
end

-- Normalized logistic so that s(0)=0 and s(1)=1 regardless of k
local function sigma(x)
	return 1.0 / (1.0 + math.exp(-x))
end
local Sa = sigma(0.5 * k)      -- σ(k/2)
local denom = (2.0 * Sa - 1.0) -- normalization denominator
local function s_norm(t)       -- t in [0,1]
	-- (σ(k*(t-0.5)) + Sa - 1) / (2*Sa - 1)
	return (sigma(k * (t - 0.5)) + Sa - 1.0) / denom
end

-- Build outer profile in XZ: points { {r, z}, ... }
local function outer_profile(rA, rB, H, samples, lipA, lipB)
	local pts = {}
	local z = 0.0

	-- bottom lip (optional straight section with radius rA)
	if lipA > 0 then
		pts[#pts + 1] = { rA, 0.0 }
		pts[#pts + 1] = { rA, math.min(lipA, H) }
		z = math.min(lipA, H)
	else
		pts[#pts + 1] = { rA, 0.0 }
	end

	-- sigmoid ramp from rA to rB over the remaining height
	local Hcore = math.max(0.0, H - z - lipB)
	if Hcore > 0 then
		for i = 1, samples - 1 do
			local t = i / samples
			local rr = rA + (rB - rA) * s_norm(t)
			local zz = z + Hcore * t
			pts[#pts + 1] = { rr, zz }
		end
		z = z + Hcore
	end

	-- top lip (optional straight)
	if lipB > 0 then
		pts[#pts + 1] = { rB, z }
		pts[#pts + 1] = { rB, H }
	else
		pts[#pts + 1] = { rB, H }
	end

	return pts
end

-- Build safe inner profile by subtracting wall from radius and clamping
local function inner_profile_from(outer_pts, wall_thickness)
	local eps = 0.15 -- minimal inner radius to avoid touching the axis
	local ip = {}
	for i = 1, #outer_pts do
		local r, z = outer_pts[i][1], outer_pts[i][2]
		local ri = math.max(eps, r - wall_thickness)
		ip[#ip + 1] = { ri, z }
	end
	return ip
end

-- Generate profiles
local pts_outer = outer_profile(r1, r2, h, n, lip1, lip2)
local pts_inner = inner_profile_from(pts_outer, wall)

-- Make profiles into XZ faces; close_to_axis=true for revolve solids
local prof_outer = poly_xz(pts_outer, false, true)
local prof_inner = poly_xz(pts_inner, false, true)

-- Revolve 360° around Z
local solid_outer = revolve(prof_outer, 360)
local solid_inner = revolve(prof_inner, 360)

-- Final shell
local adapter = difference(solid_outer, solid_inner)

emit(adapter)
-- save_stl(adapter, (__OUTDIR__ or "out") .. "/hose_adapter_sigmoid.stl")
