-- Simple example for a rod
--

local h = 700
local w = 100
local thickness = 8
local hole_height = 100
local spacer = 50

local outer = box(w, w, h)
local inner = translate(box(w - thickness, w - thickness, h), thickness / 2, thickness / 2, 0)

local hole = translate(rot_x(cylinder(8, w), 90), w / 2, w, hole_height)

local rod = difference(outer, inner)
local result = rod
for i = 1, 5 do
	result = difference(result, translate(hole, 0, 0, i * spacer))
end

-- make center
local final = translate(result, -w / 2, -w / 2, 0)

emit(final)
-- save_stl(rod, "rod.stl")
