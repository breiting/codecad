-- ISO-similar M-bolt (simplified)
-- d: diameter (mm), L: length (mm)
-- head_sw: Schlüsselweite (mm), head_h: Kopfhöhe (mm)
local function bolt_iso(d, L, opts)
	opts = opts or {}
	local head_sw = opts.head_sw or (d == 6 and 10 or d * 1.6) -- M6->SW10
	local head_h = opts.head_h or (d == 6 and 4 or d * 0.7)
	local shank_d = opts.shank_d or d
	local tip_ch = opts.tip_ch or math.min(0.5, d * 0.15)

	-- Head: Hex-Prisma + chamfer
	local head = hex_prism(head_sw, head_h)
	head = chamfer(head, tip_ch)

	-- Schaft
	local shank_len = math.max(0, L - head_h)
	local shank = cylinder(shank_d, shank_len)

	-- Zusammensetzen: Kopf oben auf Schaft
	shank = translate(shank, 0, 0, 0)
	head = translate(head, 0, 0, shank_len)

	local body = union(shank, head)

	-- Kleiner Kopf-Fillet für hübschere Kante
	body = fillet(body, math.min(0.3, d * 0.08))

	return body
end

return bolt_iso
