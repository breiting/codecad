---@meta

---@class Shape : userdata
local Shape = {}

---@class Assembly : userdata
local Assembly = {}

---@return string
function Assembly:get_name() end

---@param name string
function Assembly:set_name(name) end

---@return number @Returns the number of parts
function Assembly:count() end

---@param i number @index
---@return number @Returns the part
function Assembly:get_part(i) end

function Assembly:clear() end

---@class Part : userdata
local Part = {}

---@param name string
function Part:set_name(name) end

---@param s Shape
function Part:set_shape(s) end

---@param x number
---@param y number
---@param z number
function Part:set_explosion_vector(x, y, z) end

---@class BBox
---@field valid boolean
---@field min {x:number,y:number,z:number}
---@field max {x:number,y:number,z:number}
---@field size {x:number,y:number,z:number}
---@field center {x:number,y:number,z:number}

-- [[
-- ========================= PRIMITIVES =========================
-- ]]

---@param x number @width (X)
---@param y number @depth (Y)
---@param z number @height (Z)
---@return Shape
function box(x, y, z) end

---@param d number @diameter
---@param h number @height
---@return Shape
function cylinder(d, h) end

---@param across_flats number
---@param h number
---@return Shape
function hex_prism(across_flats, h) end

-- [[
-- ========================= TRANSFORMS =========================
-- ]]

---@param s Shape
---@param dx number @X offset
---@param dy number @Y offset
---@param dz number @Z offset
---@return Shape
function translate(s, dx, dy, dz) end

---@param s Shape
---@param deg number @degrees
---@return Shape
function rotate_x(s, deg) end

function rotate_y(s, deg) end

function rotate_z(s, deg) end

---@param s Shape
---@param factor number
---@return Shape
function scale(s, factor) end

-- [[
-- ========================= BOOLEANS =========================
-- ]]

---@param a Shape
---@param b Shape
---@param ... Shape
---@return Shape
function union(a, b, ...) end

--- Performs a boolean difference operation and removes b from a
---@param a Shape
---@param b Shape
---@return Shape
function difference(a, b) end

---@param a Shape
---@param b Shape
---@return Shape
function intersection(a, b) end

-- [[
-- ========================= FEATURES =========================
-- ]]

---@param s Shape
---@param r number @radius
---@return Shape
function fillet(s, r) end

---@param s Shape
---@param d number @distance
---@return Shape
function chamfer(s, d) end

-- [[
-- ========================= SKETCH =========================
-- ]]

---@param pts ({[1]:number,[2]:number}|{x:number,y:number})[]
---@param closed? boolean @defaults to true
---@return Shape
function poly_xy(pts, closed) end

---@param pts ({[1]:number,[2]:number}|{x:number,y:number})[]
---@param closed? boolean @defaults to false
---@param close_to_axis? boolean @if true, closes to Z-axis for revolve solids
---@return Shape
function poly_xz(pts, closed, close_to_axis) end

-- [[
-- ========================= CONSTRUCTION =========================
-- ]]

---@param face Shape
---@param height number
---@return Shape
function extrude(face, height) end

---@param profile Shape
---@param angle_deg number
---@return Shape
function revolve(profile, angle_deg) end

-- [[
-- ========================= IO =========================
-- ]]

---@param s Shape
function emit(s) end

---@param s Shape
---@param path string
function save_stl(s, path) end

---@param s Shape
---@param path string
function save_step(s, path) end

---@param asm Assembly
---@param path string
function save_assembly(asm, path) end

-- [[
-- ========================= ASSEMBLY =========================
-- ]]

---@param name string Name of the assembly
---@return Assembly
function assembly(name) end

---@param asm Assembly
---@param part Part
function add_part(asm, part) end

---@return Part
function part() end

-- [[
-- ========================= HELPER =========================
-- ]]

---@param name string
---@param default any
---@return any
function param(name, default) end

---@param v number
---@return number
function mm(v) end

---@param v number
---@return number
function deg(v) end

-- [[
-- ========================= MEASURE =========================
-- ]]

---@param s Shape
---@param useTriangulation? boolean
---@return BBox
function bbox(s, useTriangulation) end

---@param s Shape @returns a new, centered shape
---@return Shape
function center_x(s) end

---@param s Shape @returns a new, centered shape
---@return Shape
function center_y(s) end

---@param s Shape @returns a new, centered shape
---@return Shape
function center_z(s) end

---@param s Shape @returns a new, centered shape
---@return Shape
function center_xy(s) end

---@param s Shape @returns a new, centered shape
---@return Shape
function center_xyz(s) end

---@param s Shape
---@param cx number
---@param cy number
---@param cz number
---@return Shape
function center_to(s, cx, cy, cz) end

-- [[
-- ========================= GEARS =========================
-- ]]

---@param z integer
---@param module number
---@param thickness number @gear width (Z)
---@param bore number @center hole diameter
---@param pressure_deg? number @defaults to 20
---@return Shape
function gear_involute(z, module, thickness, bore, pressure_deg) end

---@type table<string, any>
PARAMS = {}

---@type string
__OUTDIR = ""
