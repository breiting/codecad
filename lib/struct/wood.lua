-- lib/struct/wood.lua
local W = {}

-- Brett/Balken: Länge L entlang X, Breite B entlang Y, Dicke T entlang Z
function W.board(L, B, T, r)
	local s = box(L, B, T)
	if r and r > 0 then
		s = chamfer(s, math.min(r, 0.45 * T))
	end
	return s
end

-- Pfosten (quadratisch) – Höhe H entlang Z
function W.post(size, H, r)
	local p = box(size, size, H)
	if r and r > 0 then
		p = chamfer(p, math.min(r, 0.45 * size))
	end
	return p
end

-- Träger (Balken) – Höhe HZ (Z), Breite HY (Y), Länge LX (X)
function W.beam(LX, HY, HZ, r)
	local b = box(LX, HY, HZ)
	if r and r > 0 then
		b = chamfer(b, math.min(r, 0.45 * math.min(HY, HZ)))
	end
	return b
end

return W
