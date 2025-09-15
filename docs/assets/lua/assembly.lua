-- Base plate
local plate = translate(center_xy(extrude(rect(80, 40), 5)), 0, 0, 30)

-- Four legs (cylinders), rotated and translated into corners
local leg = cylinder(3, 30)
local leg_positions = {
	{ -35, -15 },
	{ 35,  -15 },
	{ 35,  15 },
	{ -35, 15 },
}

local legs = {}
for _, pos in ipairs(leg_positions) do
	table.insert(legs, translate(leg, pos[1], pos[2], 0))
end

local tab = union(plate, table.unpack(legs))
tab = scale(tab, 0.2)
emit(tab)
