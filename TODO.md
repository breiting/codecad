# TODO

- `codecad init` statt `new`
- init macht auch gleich ein part mit dem gleichen namen
- Make sure to also generate a .luarc.json file in the project directory
- Camera zoom fixen
- ein und ausblenden von infinity grid mit taste
- core engine build geometrie implementieren
- loeschen von der alten main
- $LOCAL/config file

## Ideas

- Patreon
- CodeCAD as source and lua library in an extra repository?
- Animations (mutter, beilagscheibe, schraube)
- Cylinder mit abgerundeten abschluss als shape (rounded holes)
- Backend browser support?
- ccad-struct, ccad-wood, ccad-print als eigene lua libraries?
- Gewinde machen
- Project: die params einfuehren, die mann dann auch per GUI veraendern könnte
- Viewer/Camera steuern mit lua

Here are my thoughts on the next steps for this project.

- [ ] Fix gears
- [ ] Chamfer specific edges and round off corners
- [ ] Simple "extend" lengths (Sparren sollen in alle beiden Richtungen verlaengert werden)
- [ ] Fix units and degrees
- [ ] Fix outputdir
- [ ] Fix params with cli
- [ ] Think about licensing
- [ ] Import STEP
- [ ] Import DXF
- [ ] Alles als parts modellieren, dann farbe geben und gleich die exploded animation setzten. beim 3. druck quasi jedes teil auf das pad und sonst zusammengebaut.
- [ ] Texture auf oberflaechen

## Done

- [x] Update documentation and fix samples
- [x] Center option for a shape
- [x] BOM support
- [x] Build lua base library

## Paket- & Namenskonzept (zukunftssicher)

Top-Level Namespace
• ccad — alles Offizielle lebt darunter.
• Unterpakete:
• ccad.core – Grundbausteine, die immer mit CodeCAD kommen (Project/Component/Assembly, IO, deprecate utils)
• ccad.util – generische Helfer (place, transform, sketch, profile, func/math, svg/drawing, box, frame, bom)
• ccad.mech – maschinenbauliche Bauteile (bolt, nut, washer, rod, gears)
• ccad.struct– Holzbau/Struktur (wood, deck, roof)
• (optional) ccad.examples – Lernmaterial

Require-Pfade (offiziell):
local wood = require("ccad.struct.wood")
Komfort-Reexports: require("ccad.struct") kann die wichtigsten Funktionen re-exportieren (siehe unten).

Moduldateien (empfohlene Struktur)

lib/
└── ccad/
├── core/
│ ├── project.lua -- Projekt laden/speichern, Pfade, params
│ ├── component.lua -- Component-Klasse
│ ├── assembly.lua -- (statt components.lua) Collection+Ops
│ ├── io.lua -- save_stl/step wrappers falls nötig
│ └── deprecate.lua -- Deprecation-Helfer
├── util/
│ ├── transform.lua -- translate/rotate/scale + helpers
│ ├── place.lua -- array/grid/polar/along/between/stack
│ ├── sketch.lua -- poly_xy/xz, arc, circle, extrude/revolve
│ ├── profile.lua -- Standardprofile, 2D Skizzen-Bausteine
│ ├── box.lua -- box(), rounded_box(), centered helpers
│ ├── frame.lua -- einfache Rahmengeneratoren
│ ├── func.lua -- Sampling, curves, easing, sigmoid
│ ├── drawing.lua -- section_outline, projections
│ ├── svg.lua -- save svg
│ └── bom.lua -- bill of materials
├── mech/
│ ├── gears.lua -- involute gear, helpers
│ ├── bolt.lua -- ISO bolt builder
│ ├── nut.lua
│ ├── washer.lua
│ └── rod.lua
└── struct/
├── wood.lua
├── deck.lua
└── roof.lua

Deine aktuellen Dateien kannst du so verschieben/umbenennen:
• core/components.lua → ccad/core/assembly.lua
• core/component.lua → ccad/core/component.lua
• core/project.lua → ccad/core/project.lua
• util/_ → ccad/util/_ (gleich)
• mech/_ → ccad/mech/_
• struct/_ → ccad/struct/_

⸻

API-Konventionen (Benennung & Parametrisierung)

Allgemein
• snake_case für Funktionsnamen: rounded_box, section_outline, gear_involute.
• SI-Einheiten in mm (Default), Parameter heißen klar: width, depth, height; diameter/radius; thickness.
• Optionstabellen statt Positionsparameter sobald >3 Parameter oder optionale Flags:

box{ width=20, depth=15, height=10, center_xy=true }
gear_involute{ z=32, module=2, thickness=6, bore=5, pressure_deg=20 }

    •	Deterministische Defaults via param() erlauben:

local w = param("w", 20)

Transforms
• Primärfunktionen: translate(shape, x,y,z), rotate(shape, rx,ry,rz), scale(shape, s)
• Bequeme Helfer in ccad.util.transform:
move_x(shape, dx), move_xy(shape, dx,dy), rot_z(shape, deg), center_xy(shape)
→ Implementiert auf den Primärfunktionen.

Placement
• array(fn, n, dx) – 1D
• grid(fn, nx, sx, ny, sy) – 2D
• polar(fn, n, r, start_deg?) – kreisförmig
• between(a, b, n, fn) – linear verteilt
• along(path_points, fn, params?) – entlang 2D/3D Pfad
Alle liefern union der erzeugten Shapes oder Liste (per Option collect=true).

Sketch/Profiles
• 2D primitives: poly_xy(points, closed?), arc_xy(cx,cy,r, a0,a1), circle_xy(r)
• Solids: extrude(face, h), revolve(profile, axis?, angle?)
• Profile-Bausteine: rect_profile, rounded_rect_profile{w,h,r} etc.

Booleans & Features
• union(a,b,...), difference(a,b,...), intersect(a,b)
• fillet(shape, radius, filter?), chamfer(shape, dist, filter?)
• filter als Funktor oder ccad.util.select.edges_by_axis("z+"), etc.

⸻

Versionierung & Deprecation

Version
• Globale Version:

-- ccad/init.lua
local M = {}
M.VERSION = "1.0.0"
return M

    •	Pro Subpaket optional:

-- ccad/util/init.lua
return { VERSION = "1.0.0" }

Deprecation Utility
• ccad.core.deprecate.warn(old_name, new_name, remove_in)
• Shim-Beispiel (alte Namen weiterreichen):

-- ccad/util/place.lua (Shim-Ecke oben)
local dep = require("ccad.core.deprecate")
function place.grid(...)
dep.warn("place.grid", "ccad.util.place.grid", "2.0")
return grid(...)
end

⸻

Dokumentation/Generierung

Kommentierstil (EmmyLua kompatibel)

--- Create a rounded box solid.
-- @param opts table Options: { width, depth, height, radius, center_xy? }
-- @return shape Solid OCCT shape
function rounded_box(opts) ... end

Tooling
• LDoc zum Generieren nach Markdown/HTML (einfach, Lua-nativ).
• GitHub Pages: /docs als Output-Ziel, Startseite mit Tutorial + API Sidebar.
• Cheatsheet: docs/cheatsheet.md (eine Seite), plus PDF Export.

⸻

Re-Exports (komfortable Requires)

Damit Nutzer nicht zu tiefe Pfade tippen müssen:

-- ccad/struct/init.lua
local M = {}
local wood = require("ccad.struct.wood")
local deck = require("ccad.struct.deck")
local roof = require("ccad.struct.roof")
-- re-export gängige Funktionen:
M.wood = wood
M.deck = deck
M.roof = roof
return M

Analog für ccad.util und ccad.mech.

⸻

Migrations-Map (konkret für deine Files)

Bisher Neu (empfohlen) Anmerkung
core/component.lua ccad/core/component.lua ggf. Klasse Component mit :shape(), :transform(), :material()
core/components.lua ccad/core/assembly.lua „Assembly“ statt „components“ (klarer)
core/project.lua ccad/core/project.lua Load/Save Project, Params, Paths
mech/gears.lua ccad/mech/gears.lua API: gear_involute{...}
mech/bolt.lua ccad/mech/bolt.lua bolt{size="M6", length=..., head="hex"}
mech/nut.lua ccad/mech/nut.lua nut{size="M6", height=...}
mech/washer.lua ccad/mech/washer.lua
mech/rod.lua ccad/mech/rod.lua
struct/wood.lua ccad/struct/wood.lua
struct/deck.lua ccad/struct/deck.lua
struct/roof.lua ccad/struct/roof.lua
util/box.lua ccad/util/box.lua box{...}, rounded_box{...}, centered
util/place.lua ccad/util/place.lua array/grid/polar/along/between
util/transform.lua ccad/util/transform.lua center_xy, move_x, rot_z, …
util/sketch.lua ccad/util/sketch.lua poly/arc/extrude/revolve
util/profile.lua ccad/util/profile.lua Standardprofile
util/func.lua ccad/util/func.lua sampling, easing, curves
util/drawing.lua ccad/util/drawing.lua section_outline_x/y/z, svg helpers
util/svg.lua ccad/util/svg.lua
util/frame.lua ccad/util/frame.lua Rahmen/Hilfsgeometrie
util/bom.lua ccad/util/bom.lua BOM aus Assembly

⸻

Mini-Beispiele (API-Pattern)

Rounded Box (Optionstabelle + Defaults)

local tr = require("ccad.util.transform")

--- @param opts {width:number, depth:number, height:number, radius:number, center_xy?:boolean}
local function rounded_box(opts)
local w = opts.width or 20
local d = opts.depth or 20
local h = opts.height or 10
local r = opts.radius or 2

local face = ccad.util.sketch.rounded_rect_xy{ width=w, height=d, radius=r }
local s = extrude(face, h)
if opts.center_xy then s = tr.center_xy(s) end
return s
end

return { rounded_box = rounded_box }

Polar Placement (wiederverwendbar)

-- ccad/util/place.lua
local function polar(n, r, make)
local acc = nil
for i=0,n-1 do
local a = 360 \* i / n
local sh = make(i, a)
sh = translate(sh, r, 0, 0)
sh = rot_z(sh, a)
acc = acc and union(acc, sh) or sh
end
return acc
end

return { polar = polar, ... }

⸻

Vorgehen (pragmatisch) 1. Ordner anlegen & Dateien verschieben wie oben. 2. Re-exports in ccad/util/init.lua, ccad/mech/init.lua, ccad/struct/init.lua. 3. Deprecation-Shims für alte Require-Pfade (optional 1 Release lang). 4. Docs: LDoc Setup + GitHub Pages. 5. Cheatsheet schreiben (A4), Tutorials in tutorials/.

Wenn du magst, schicke ich dir als nächsten Schritt:
• ein fertiges ccad/core/deprecate.lua
• ein ccad/util/init.lua mit Re-exports
• LDoc-Konfig + erster API-Header für z. B. rounded_box

Sag Bescheid, ob du Re-exports und Deprecation sofort willst, dann schreibe ich dir die konkreten Lua-Snippets zum Einchecken.
