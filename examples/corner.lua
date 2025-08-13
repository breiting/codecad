-- ===== Corner Guard (parametrisierbar, max 40 mm) =====

local sz = 40 -- Außenkante (<= 40)
local th = 2  -- Wandstärke
local hd = 6  -- Lochdurchmesser (Zentrum), 0 = keins

-- Helpers
local function plate_xy(w, h, t, hole_d)
	local p = box(w, h, t)
	if hole_d and hole_d > 0 then
		-- Loch minimal clampen, damit’s nicht in die Wand reinragt
		local max_hole = math.max(0, math.min(w, h) - 2 * t)
		local d = math.min(hole_d, max_hole)
		if d > 0 then
			local hcut = cylinder(d, t + 0.2)
			p = difference(p, translate(hcut, w / 2, h / 2, -0.1))
		end
	end
	return p
end

-- Gleiche Platte, in YZ aufgestellt (am Ursprung anliegend)
local function plate_yz(w, h, t, hole_d)
	return rot_x(plate_xy(w, h, t, hole_d), 90)
end

-- Gleiche Platte, in XZ aufgestellt, sauber an XY bündig (mit Überlappung um th)
local function plate_xz(w, h, t, hole_d)
	-- +t in h, damit die drei Platten sich „verzahnen“ und keine Lücke entsteht
	local p = plate_xy(w, h + t, t, hole_d)
	p = rot_y(p, -90)
	return translate(p, 0, -t, 0)
end

-- Baukasten: drei Seiten (XY, YZ, XZ)
local function corner_guard(size, t, hole_d)
	local s1 = plate_xy(size, size, t, hole_d)
	local s2 = plate_yz(size, size, t, hole_d)
	local s3 = plate_xz(size, size, t, hole_d)
	local g = union(s1, s2, s3)
	return g
end

local corner = corner_guard(sz, th, hd)

emit(corner)

save_step(corner, "out/corner.step")
