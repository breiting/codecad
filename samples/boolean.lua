-- boolean.lua
--
local base = box(1000, 500, 50)
local cut = box(200, 200, 50)
cut = translate(cut, 100, 0, 0)

local result = difference(base, cut)
emit(result)
save_stl(result, "boolean.stl")
-- save_step(result, "boolean.step")
