-- examples/simple.lua
-- Minimal MVP example: create two boxes, union them, emit and save to STL/STEP

local a = box(500, 200, 100)
local b = box(100, 250, 200)
local c = cylinder(100, 500)

b = fillet(b, 1)
a = chamfer(a, 1)

local model = union(a, b, c)

emit(model)
-- save_stl(model, "hello_box.stl")
-- Optional STEP for CAD exchange
-- save_step(model, "hello_box.step")
print("Done.")
