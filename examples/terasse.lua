local util = require("util.place")
local W = require("struct.wood")
local Deck = require("struct.deck")
local Roof = require("struct.roof")

-- Param
local Lx = param("Lx", 4000)      -- Länge Terrasse (X)
local Ly = param("Ly", 2500)      -- Tiefe Terrasse (Y)
local H = param("H", 400)         -- Höhe Unterkonstruktion (Z bis Oberkante Balken)
local gap = param("gap", 10)      -- Spalt zwischen Dielen
local Bw = param("Bw", 100)       -- Diele Breite
local Bt = param("Bt", 30)        -- Diele Dicke
local By = param("By", 60)        -- Balken Breite
local Bz = param("Bz", 100)       -- Balken Höhe
local NB = param("NB", 5)         -- Anzahl Balken (Unterkonstruktion)
local postS = param("postS", 160) -- Pfosten 16x16 cm
local postH = param("postH", 2200)
local pitch = param("pitch", 10)  -- Dachneigung Grad
local ax = param("ax", 400)       -- Sparrenabstand

-- Unterkonstruktion (auf Z=0)
local sub = Deck.substructure(Lx, Ly, By, Bz, NB)

-- Dielen (auf Höhe H) – leicht abgeschrägt
local deck = Deck.decking(Lx, Ly, Bw, Bt, gap, 0.8)
deck = translate(deck, 0, 0, H)

-- Pfostenraster z. B. 3x2
local posts = Deck.posts_grid(3, 2, Lx / 2, Ly, postS, postH)

-- Querträger oben auf den Pfosten (einfach)
local beamTop = W.beam(Lx, By, Bz, 0.5)
beamTop = translate(beamTop, 0, 0, postH)

-- Einfache Dachkonstruktion
local rafters = Roof.rafters(Lx, Ly, pitch, ax, By, Bz)
rafters = translate(rafters, 0, 0, postH + Bz)

-- Seitenbretter (optional): z. B. Lamellen an einer Seite
local lam_w, lam_t, lam_gap = 80, 15, 20
local lam_h = postH
local count = math.floor((lam_h + lam_gap) / (lam_t + lam_gap))
local lam = {}
for i = 0, (count - 1) do
	local b = W.board(Lx, lam_w, lam_t, 0.5)
	b = rot_x(b, 90) -- hochkant
	b = translate(b, 0, -0.5 * Ly + lam_w / 2, i * (lam_t + lam_gap))
	lam[#lam + 1] = b
end
local slats = union(table.unpack(lam))

-- Zusammensetzen
local model = union(sub, deck, posts, beamTop, rafters, slats)
emit(model)
