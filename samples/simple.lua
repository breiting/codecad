-- simple.lua
-- Minimal example: create some basic shapes

local a = box(50, 50, 200)
local b = box(10, 20, 100)
local c = cylinder(10, 50)

b = translate(b, 100, 0, 0)
c = translate(c, 0, 100, 0)

b = fillet(b, 1)
a = chamfer(a, 1)

local model = union(a, b, c)

save_stl(model, "out/simple.stl")
print("Done.")
