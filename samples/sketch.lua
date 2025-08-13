local p = require("util.profile")

local rect = p.rect(100, 100)
save_stl(rect, "out/rect.stl")
-- Extrude the same rectangle
save_stl(extrude(rect, 10), "out/extruded.stl")

-- Revolve points
local r1 = 10
local pts = {
	{ r1,      0 },
	{ r1 + 5,  10 },
	{ r1 + 10, 20 },
	{ r1,      30 },
}
local profile = poly_xz(pts, false, true)
save_stl(revolve(profile, 180), "out/revolve.stl")
