-- Wall bench with posts, sills ("Mauerbank"), and rafters
-- Params (mm unless noted):
--   post_size : square post section (default 160)
--   post_h    : post height
--   span_x    : clear span in X between left/right posts (2 posts → 4 total)
--   span_y    : clear span in Y between front/back posts
--   sill_by   : sill width (Y)
--   sill_bz   : sill height (Z)
--   r_count   : number of rafters (along X)
--   r_by      : rafter width (Y)  [e.g., 100 = 10 cm]
--   r_bz      : rafter height (Z) [e.g., 160 = 16 cm]
--   edge      : small chamfer for nicer print edges

local wood = require("struct.wood")
local place = require("util.place")

-- -------- Parameters --------
-- Stützen
local post_size = param("post_size", 160)
local post_h = param("post_h", 2200)

local span_x = param("span_x", 2000)
local span_y = param("span_y", 2000)

-- Mauerbank
local sill_by = param("sill_by", 160)
local sill_bz = param("sill_bz", 160)

local r_count = math.max(2, param("r_count", 5))
local r_by = param("r_by", 100) -- 10 cm
local r_bz = param("r_bz", 160) -- 16 cm

local edge = param("edge", 2.0)

-- -------- Builders --------
local function make_post()
	-- square post: size x size x height, positioned at local (0,0,0)
	return wood.post(post_size, post_h, edge)
end

local function make_sill()
	-- sill (mauerbank) runs along X and rests on top of posts (z = post_h)
	-- length along X: span_x
	-- width  along Y: sill_by
	-- height along Z: sill_bz
	return wood.beam(span_x, sill_by, sill_bz, edge)
end

local function make_rafter()
	-- rafters run along Y (from front sill to back sill)
	-- length along X-parameter is used for "along axis"; here along Y → we pass span_y as "length"
	return place.place(wood.beam(span_y, r_by, r_bz, edge), r_by, 0, 0, 0, 0, 90)
end

-- -------- Assembly --------

-- 1) Posts: 2 x 2 grid
-- positions: (0,0), (span_x,0), (0,span_y), (span_x,span_y)
local posts = place.grid(make_post, 2, span_x - post_size, 2, span_y - post_size)

-- 2) Sills: two beams along X, sitting on top of the two rows of posts
-- y = 0 (front) and y = span_y (back), z = post_h
local sill_front = place.place(make_sill(), 0, 0, post_h)
local sill_back = place.place(make_sill(), 0, span_y - post_size, post_h)
local sills = union(sill_front, sill_back)

-- 3) Rafters: r_count pieces along X, evenly spaced, resting on the two sills
-- Each rafter runs along Y (length = span_y), at z = post_h + sill_bz
local rafters =
	translate(place.grid(make_rafter, r_count, (span_x - r_by) / (r_count - 1), 2, 0), 0, 0, post_h + post_size)

-- -------- Result --------
local model = union(posts, sills, rafters)
save_stl(model, "out/mauerbank_with_rafters.stl")
