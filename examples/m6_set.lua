package.path = "../lib/?.lua;../lib/?/init.lua;../lib/?/?.lua;" .. package.path

local bolt = require("mech.bolt")
local nut = require("mech.nut")
local washer = require("mech.washer")

-- M6 Defaults
local M = 6
local L = 25

local b = bolt(M, L, { head_sw = 10, head_h = 4 })
local n = nut(M, { sw = 10, h = 5 })
local w = washer(6.4, 12, 1.6)

-- Anordnen (exploded view)
n = translate(n, 0, 0, L + 8)
w = translate(w, 0, 0, L + 4)

local set = union(b, n, w)
emit(set) -- Auto STL/STEP nach out/
-- save_step(set, "out/m6_set.step")
