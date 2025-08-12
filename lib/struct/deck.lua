-- lib/struct/deck.lua
local util = require("util.place")
local W = require("struct.wood")

local D = {}

-- Unterkonstruktion: n Beams entlang Y, Länge Lx, Breite Ly, Balkenmaß By x Bz, spacing Sy
function D.substructure(Lx, Ly, beam_y, beam_z, n_beams)
	local spacing = (n_beams > 1) and ((Ly - beam_y) / (n_beams - 1)) or 0
	local function make_beam()
		return W.beam(Lx, beam_y, beam_z, 0.5)
	end
	return util.array(make_beam, n_beams, 0, 1, spacing, 0)
end

-- Dielenlage: Bretter entlang X (Lx), Brettbreite Bw (Y), Dicke Bt (Z), Abstand gap (Y), Gesamtbreite Ly
function D.decking(Lx, Ly, Bw, Bt, gap, edge_ch)
	local n = math.floor((Ly + gap) / (Bw + gap))
	local used = n * (Bw + gap) - gap
	local offset = (Ly - used) * 0.5 -- zentrieren
	local function make_board()
		return W.board(Lx, Bw, Bt, edge_ch or 0.5)
	end
	local parts = {}
	for i = 0, (n - 1) do
		parts[#parts + 1] = util.place(make_board(), 0, offset + i * (Bw + gap), 0)
	end
	return union(table.unpack(parts))
end

-- Pfostenraster
function D.posts_grid(nx, ny, spacing_x, spacing_y, size, H)
	local function mk()
		return W.post(size, H, 0.5)
	end
	return util.array(mk, nx, spacing_x, ny, spacing_y, 0)
end

return D
