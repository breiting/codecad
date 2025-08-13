-- Spur Gear (Trapez-Zahn-Approx)
-- Parameter:
--  z       : Zähnezahl
--  od      : Außendurchmesser (mm)
--  th      : Zahnbreite / Dicke (mm, in Z)
--  bore    : Bohrungsdurchmesser (mm)
--  tooth_f : Faktor für Zahndicke (0.85..1.05); 0.95 vermeidet Überlappungen
--
-- Nutzung:
--   ccad build examples/gears.lua -o out -D z=24 -D od=60 -D th=8 -D bore=6
--
local z = math.max(6, param("z", 24))
local od = math.max(5, param("od", 60))
local th = math.max(2, param("th", 8))
local bore = math.max(0, param("bore", 6))
local tooth_f = param("tooth_f", 0.95) -- leicht schmaler für saubere Union

-- abgeleitete Standardgrößen (Vollzahnprofil genähert):
-- Modul m = (Do)/(z + 2)  => Do = (z+2)*m
local m = od / (z + 2)
local add = 1.0 * m                    -- Addendum
local ded = 1.25 * m                   -- Dedendum
local rp = 0.5 * z * m                 -- Teilkreisradius
local ro = rp + add                    -- Kopfkreisradius (Außen)
local rr = math.max(rp - ded, 0.2 * m) -- Fußkreisradius (Root)

-- Zahndicke auf Teilkreis: t_p = π*m/2
-- Wir nähern die tangentiale Breite mit einer Sehne am Teilkreis
local tp = math.pi * m * 0.5 * tooth_f
local chord = 2 * rp * math.sin(tp / (2 * rp))

-- Radiale Zahnhöhe (Blocktiefe)
local depth = ro - rr

-- Grundkörper (Fußkreis-Zylinder)
local base = cylinder(2 * rr, th)

-- Zahn als Rechteckblock: X = radial (depth), Y = tangential (chord), Z = th
local tooth = box(depth, chord, th)
-- an den Fußkreis schieben und Y zentrieren
tooth = translate(tooth, rr, -chord / 2, 0)

-- Alle Zähne verteilen
local teeth = {}
for i = 0, z - 1 do
	local a = (360 / z) * i
	teeth[#teeth + 1] = rot_z(tooth, a)
end

-- Zusammenbauen
local gear = union(base, table.unpack(teeth))

-- Bohrung
if bore > 0 then
	local hole = translate(cylinder(bore, th + 0.2), 0, 0, -0.1)
	gear = difference(gear, hole)
end

-- leichte Kantenbrüche für Druckfreundlichkeit
gear = chamfer(gear, math.min(0.35 * m, 0.6))

emit(gear)
