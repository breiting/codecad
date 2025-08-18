-- lib/struct/roof.lua
local W = require("struct.wood")

local R = {}

-- Einfaches Sparrendach: Rechteckiger Grundriss Lx x Ly, Neigung deg, Sparrenabstand ax, Sparrenmaß sy x sz
function R.rafters(Lx, Ly, pitch_deg, ax, spar_y, spar_z)
	local pitch = math.rad(pitch_deg)
	local rise = 0.5 * Ly * math.tan(pitch) -- Höhe am First
	local span = Lx
	local len = math.sqrt((0.5 * Ly) ^ 2 + rise ^ 2)

	local function sparren()
		local s = W.beam(len, spar_y, spar_z, 0.5)
		s = rot_x(s, 90)           -- z->y umlegen optisch
		s = rot_z(s, pitch_deg)    -- Neigung
		s = translate(s, 0, 0.5 * Ly, 0) -- um First drehen
		return s
	end

	local count = math.floor((span + ax / 2) / ax)
	local parts = {}
	for i = 0, (count - 1) do
		local x = -0.5 * span + i * ax
		parts[#parts + 1] = translate(sparren(), x, 0, 0)
	end
	return union(table.unpack(parts))
end

return R
