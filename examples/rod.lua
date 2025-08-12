-- Simple example for a rod
--

local outer = box(70, 70, 500)
local inner = translate(box(66, 66, 500), 2, 2, 0)

local rod = difference(outer, inner)

emit(rod)
save_stl(rod, "rod.stl")
