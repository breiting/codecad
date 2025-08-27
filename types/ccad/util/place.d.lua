---@meta
---@module "ccad.util.place"

---@class Shape

---@alias Make1DFn fun(i:integer, j:integer): Shape
---@alias MakeGridFn fun(i:integer, j:integer): Shape
---@alias MakePolarFn fun(i:integer, angle_deg:number): Shape

local P = {}

--- Place a shape with optional uniform scale and rotations, then translate.
--- Order: scale → rotX → rotY → rotZ → translate.
---@param s Shape
---@param x? number
---@param y? number
---@param z? number
---@param rx? number
---@param ry? number
---@param rz? number
---@param sc? number
---@return Shape
function P.place(s, x, y, z, rx, ry, rz, sc) end

--- 1D array along X; optionally replicated along Y (grid).
--- Calls maker(i,j) with 0-based indices.
---@param make Make1DFn
---@param nx integer
---@param dx number
---@param ny? integer
---@param dy? number
---@return Shape
function P.array(make, nx, dx, ny, dy) end

--- 2D grid (X/Y) using a maker function `(i, j) -> Shape`.
---@param make MakeGridFn
---@param nx integer
---@param dx number
---@param ny integer
---@param dy number
---@return Shape
function P.grid(make, nx, dx, ny, dy) end

--- Place `n` instances around a circle of radius `r` in the XY-plane.
--- Calls maker(i, angle_deg).
---@param n integer
---@param r number
---@param make MakePolarFn
---@param angle0? number
---@return Shape
function P.polar(n, r, make, angle0) end

return P
