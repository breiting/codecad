-- Base plate
local plate = center_xy(extrude(rect(80, 40), 6))

-- Cylindrical boss
local boss = translate(cylinder(10, 20), 0, 0, 6)

-- Combine
local raw = union(plate, boss)

-- Drill a hole in the boss
local hole = center_xy(cylinder(4, 30))
local bracket = difference(raw, hole)

bracket = scale(bracket, 0.5)
emit(bracket)
