-- examples/hose_adapter.lua
-- Zwei Durchmesser mit sanfter Kurve verbinden; Profil im XZ (r,z), revolve 360°.
local d1 = param("d1", 35)    -- unten
local d2 = param("d2", 50)    -- oben
local L = param("L", 40)      -- Länge
local t = param("t", 2.4)     -- Wand
local rb = param("blend", 10) -- Kurvenlänge im Profil

-- Außenprofil (Radien)
local r1 = 0.5 * d1
local r2 = 0.5 * d2

print(r1)
print(r2)

-- Profilpunkte (r,z) – 2 Linien + 1 Übergangsbogen (hier als 2 Segmente genähert)
local pts = {
	{ r1,                   0 },   -- unten außen
	{ r1,                   rb },  -- gerade
	{ r1 + (r2 - r1) * 0.5, L - rb }, -- Übergang
	{ r2,                   L - rb },
	{ r2,                   L },   -- oben außen
}

-- Außenface schließen zur Achse, damit Revolve Solid wird
local outer = poly_xz(pts, false, true)
local shell = revolve(outer, 360)

-- Innennegativ (gleiche Punkte minus Wand)
local r1i = math.max(0, r1 - t)
local r2i = math.max(0, r2 - t)
local pts_i = {
	{ r1i,                     0 },
	{ r1i,                     rb },
	{ r1i + (r2i - r1i) * 0.5, L - rb },
	{ r2i,                     L - rb },
	{ r2i,                     L },
}
local inner = revolve(poly_xz(pts_i, false, true), 360)

local adapter = difference(shell, inner)
emit(chamfer(adapter, math.min(0.6, 0.45 * t)))
