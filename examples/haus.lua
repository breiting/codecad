local wall_thickness = param("wall_thickness", 50)
local house_length = param("house_length", 6000)
local house_width = param("house_width", 4000)
local house_height = param("house_height", 3000)
local door_width = param("door_width", 1000)
local door_height = param("door_height", 2100)
local window_width = param("window_width", 800)
local window_height = param("window_height", 1200)
local terrace_depth = param("terrace_depth", 2000)
local round_window_diameter = param("round_window_diameter", 600)

local function create_wall(length, height)
  return box(length, wall_thickness, height)
end

local function create_door()
  return difference(box(door_width, wall_thickness, door_height), translate(box(door_width - 20, wall_thickness + 1, door_height - 20), 0, 0, 0))
end

local function create_window()
  return difference(box(window_width, wall_thickness, window_height), translate(box(window_width - 20, wall_thickness + 1, window_height - 20), 0, 0, 0))
end

local walls = union(
  translate(create_wall(house_length, house_height), 0, 0, 0), -- Front wall
  translate(create_wall(house_length, house_height), 0, house_width - wall_thickness, 0), -- Back wall
  translate(create_wall(wall_thickness, house_width, house_height), 0, 0, 0), -- Left wall
  translate(create_wall(wall_thickness, house_width, house_height), house_length - wall_thickness, 0, 0) -- Right wall
)

local door = translate(create_door(), (house_length - door_width) / 2, 0, 0)
local window1 = translate(create_window(), (house_length - window_width) / 2, wall_thickness, (house_height - window_height) / 2)
local window2 = translate(create_window(), (house_length - window_width) / 2, house_width - wall_thickness, (house_height - window_height) / 2)

local round_window1 = translate(cylinder(round_window_diameter, wall_thickness), 0, house_width / 2, (house_height - round_window_diameter) / 2)
local round_window2 = translate(cylinder(round_window_diameter, wall_thickness), house_length - wall_thickness, house_width / 2, (house_height - round_window_diameter) / 2)

local terrace = translate(box(house_length, terrace_depth, wall_thickness), 0, house_width, -wall_thickness)

emit(union(walls, door, window1, window2, round_window1, round_window2, terrace))
