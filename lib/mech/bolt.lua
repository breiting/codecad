-- ISO-ähnliche M-Schraube (vereinfachtes Gewinde standardmäßig AUS)
-- d: Nenndurchmesser (mm), L: Gesamtlänge (mm)
-- head_sw: Schlüsselweite (mm), head_h: Kopfhöhe (mm)
-- shank_clearance: optional, falls du bewusst kleiner drucken willst
-- thread: false|true (optisches Gewinde später)
local function bolt_iso(d, L, opts)
	opts = opts or {}
	local head_sw = opts.head_sw or (d == 6 and 10 or d * 1.6) -- M6->SW10
	local head_h = opts.head_h or (d == 6 and 4 or d * 0.7) -- grobe Näherung
	local shank_d = opts.shank_d or d
	local tip_ch = opts.tip_ch or math.min(0.5, d * 0.15)   -- Kantenbruch
	local thread = opts.thread or false

	-- Kopf: Hex-Prisma + kleiner Kantenbruch (chamfer)
	local head = hex_prism(head_sw, head_h)
	head = chamfer(head, tip_ch)

	-- Schaft
	local shank_len = math.max(0, L - head_h)
	local shank = cylinder(shank_d, shank_len)

	-- Zusammensetzen: Kopf oben auf Schaft
	shank = translate(shank, 0, 0, 0)
	head = translate(head, 0, 0, shank_len)

	local body = union(shank, head)

	-- (Optional) optisches Gewinde – später via Helix sweep
	-- if thread then body = union(body, _make_visual_thread(d, pitch, shank_len)) end

	-- Kleiner Kopf-Fillet für hübschere Kante
	body = fillet(body, math.min(0.3, d * 0.08))

	return body
end

return bolt_iso
