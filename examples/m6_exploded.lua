local bolt = require("mech.bolt")
local nut = require("mech.nut")
local washer = require("mech.washer")

local M, L = 6, 25
local b = bolt(M, L, { head_sw = 10, head_h = 4 })
local n = nut(M, { sw = 10, h = 5 })
local w = washer(6.4, 12, 1.6)

-- Positionierung in assembled state
-- (wir stapeln entlang Z)
n = translate(n, 0, 0, L + 6)
w = translate(w, 0, 0, L + 3)

-- Assembly + Explode-Vektoren (Richtung/Ziel im exploded state)
local asm = assembly("m6_set")
add_part(asm, part("bolt", b, -50, 0, 0))   -- fährt nach unten
add_part(asm, part("nut", n, 0, 0, 25))     -- nach oben
add_part(asm, part("washer", w, 0, 30, 15)) -- schräg weg

-- Einzel-STL/STEP + Manifest schreiben
emit_assembly(asm, "out")
