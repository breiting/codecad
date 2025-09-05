local T = require("util.transform")
local B = require("util.box")

-- --- Inputs (constraints) ---
local L_left = param("L_left", 120)   -- mm
local L_right = param("L_right", 120) -- mm
local spacing = param("D", 200)       -- distance between hole centers (constraint)
local wide = param("wide", 20)        -- bar width
local thick = param("thick", 6)       -- extrusion thickness
local hole_d = param("hole_d", 8)
local hole_recession = param("hole_recession", 10)

local function make_handle(len, height, thickness)
	local b = box(len, height, thickness)
	local hole = cylinder(hole_d, thick)
	hole = T.move_y(hole, height / 2)
	hole = T.move_x(hole, len - hole_recession)
	b = difference(b, hole)
	return b
end

local function calc_theta(a, b)
	local c = spacing
	if a + b <= c or a + c <= b or b + c <= a then
		error("Ungültige Seitenlängen – kein Dreieck möglich.")
	end

	-- Kosinus berechnen
	local cosGamma = (a ^ 2 + b ^ 2 - c ^ 2) / (2 * a * b)

	-- Numerische Ungenauigkeiten abfangen
	if cosGamma > 1 then
		cosGamma = 1
	end
	if cosGamma < -1 then
		cosGamma = -1
	end

	local gamma = math.acos(cosGamma) -- Ergebnis in Radiant
	return gamma * (180 / math.pi)
end

local left = T.move_y(T.rot_z(make_handle(L_left, wide, thick), 180), wide)
local right = make_handle(L_right, wide, thick)

-- calculate angle theta
local theta
theta = calc_theta(L_left - hole_recession, L_right - hole_recession)

theta = 180 - theta
print(theta)

left = T.rot_z(left, -theta / 2)
right = T.rot_z(right, theta / 2)

local b = T.move_y(B.rounded_shell(100, 50, 20, 4, 5), -70)

local shape = union(left, right, b)

emit(shape)
