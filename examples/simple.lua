-- examples/hello_box.lua
-- Minimal MVP example: create two boxes, union them, emit and save to STL/STEP

local a = box(50, 20, 10)
local b = box(10, 30, 20)
local c = cylinder(10, 50)

-- Shift b by 20 in X by stacking with another box (MVP lacks translate; just union overlap)
local model = union(a, b, c)

emit(model)
save_stl(model, "hello_box.stl")
-- Optional STEP for CAD exchange
-- save_step(model, "hello_box.step")
print("Done.")
