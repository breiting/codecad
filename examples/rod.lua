-- Simple example for a rod
--

local h = 700
local w = 100
local thickness = 4

local outer = box(w, w, h)
local inner = translate(box(w - thickness, w - thickness, h), thickness / 2, thickness / 2, 0)

local rod = difference(outer, inner)

rod = translate(rod, -w / 2, -w / 2, 0)

emit(rod)
-- save_stl(rod, "rod.stl")
