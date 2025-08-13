-- Sechskantmutter ISO-ähnlich
-- d: Nenndurchmesser (mm)
-- h: Höhe (mm), sw: Schlüsselweite (mm), hole: Kerndurchmesser (mm)
local function nut_iso(d, opts)
	opts = opts or {}
	local sw = opts.sw or (d == 6 and 10 or d * 1.6)
	local h = opts.h or (d == 6 and 5 or d * 0.8)
	local hole = opts.hole or (d - 0.1) -- leicht kleiner -> später drucktoleranz
	local cham = opts.cham or math.min(0.5, d * 0.15)

	local body = hex_prism(sw, h)
	local bore = cylinder(hole, h + 0.2)
	bore = translate(bore, 0, 0, -0.1)

	local nut = difference(body, bore)
	nut = chamfer(nut, cham) -- Kanten brechen
	return nut
end

return nut_iso
