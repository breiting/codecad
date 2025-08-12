---@meta

---@class Shape : userdata
local Shape = {}

---@param v number : millimeters
---@return number
function mm(v) end

---@param v number : degrees
---@return number
function deg(v) end

---@param name string
---@param default any
---@return any
function param(name, default) end

---@param x number @mm
---@param y number @mm
---@param z number @mm
---@return Shape
function box(x, y, z) end

---@param d number Diameter
---@param h number Height
---@return Shape
function cylinder(d, h) end

---@param across_flats number across flats
---@param h number Height
---@return Shape
function hex_prism(across_flats, h) end

---@param a Shape
---@param b Shape
---@return Shape
function difference(a, b) end

---@param s Shape
---@param r number Radius for fillet in mm
---@return Shape
function fillet(s, r) end

---@param s Shape
---@param d number Distance for chamfer in mm
---@return Shape
function chamfer(s, d) end

---@param ... Shape
---@return Shape
function union(...) end

---@param s Shape
---@param dx number @mm
---@param dy number @mm
---@param dz number @mm
---@return Shape
function translate(s, dx, dy, dz) end

---@param s Shape @degrees
---@param a number
---@return Shape
function rotate_x(s, a) end

---@param s Shape @degrees
---@param a number
---@return Shape
function rotate_y(s, a) end

---@param s Shape @degrees
---@param a number
---@return Shape
function rotate_z(s, a) end

---@param s Shape
---@param f number
---@return Shape
function scale(s, f) end

---@param s Shape
---@param name? string
function emit(s, name) end

---@param s Shape
---@param path string
function save_stl(s, path) end

---@param s Shape
---@param path string
function save_step(s, path) end

---@type table<string, any>
PARAMS = {}

---@type string
__OUTDIR = ""
