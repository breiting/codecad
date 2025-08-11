-- examples/ops.lua
--
local base = box(1000, 500, 50)
local cut = box(200, 200, 50)
cut = translate(cut, 100, 0, 0)
-- cut = rotate_z(cut, 20)

local result = difference(base, cut)
emit(result)
save_stl(result, "ops.stl")
-- save_step(result, "__OUTDIR__/ops_demo.step")
