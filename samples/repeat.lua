local place = require("util.place")

local board_height = 24
local board_width = 100
local board_length = 1000
local spacing = 20
local boards = 8

local function make_board()
	return box(board_width, board_length, board_height)
end

local function make_square()
	return box(board_width, board_width, board_width)
end

save_stl(place.array(make_board, boards, board_width + spacing), "out/array.stl")
save_stl(place.grid(make_square, boards, board_width + spacing, boards, board_width + spacing), "out/grid.stl")
