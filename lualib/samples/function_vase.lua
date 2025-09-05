-- examples/function_vase.lua
local func = require("util.func")

-- r(θ) = R * (1 + k*cos(nθ)) with vertical shaping
local R = param("R", 40)  -- base radius
local k = param("k", 0.2) -- ripple strength
local n = param("n", 6)   -- number of lobes
local H = param("H", 120) -- height
local thick = param("thick", 2.4)

-- Build profile in XZ: z goes 0..H, radius varies with z
local steps = 200
local rz = {}
for i = 0, steps do
	local t = i / steps
	local z = H * t
	local theta = 2 * math.pi * n * t
	local r = R * (1 + k * math.cos(theta)) * (0.3 + 0.7 * t) -- tapering
	rz[#rz + 1] = { r, z }
end

local prof = poly_xz(rz, false, true) -- close_to_axis = true for revolve solid
local shell_outer = revolve(prof, 360)

-- inner shell (wall thickness)
local rz_i = {}
for i = 1, #rz do
	rz_i[i] = { math.max(0, rz[i][1] - thick), rz[i][2] }
end
local prof_i = poly_xz(rz_i, false, true)
local shell_inner = revolve(prof_i, 360)

emit(difference(shell_outer, shell_inner))
