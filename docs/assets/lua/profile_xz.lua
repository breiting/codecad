-- Bottle-like shape by revolving an open XZ profile around Z.
-- X is radius, Z is height. Start at the axis (x=0) for a sealed top/bottom.
local outline = profile_xz({
	{ 0,  0 }, -- on axis
	{ 25, 0 }, -- outer radius at base
	{ 28, 5 },
	{ 30, 15 },
	{ 20, 3 },
	{ 18, 50 },
	{ 10, 60 },
	{ 0,  70 },
}, true)

local body = revolve(outline, 360)
body = scale(body, 0.5)
emit(body)
