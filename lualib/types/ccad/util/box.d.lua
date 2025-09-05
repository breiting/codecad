---@meta
---@module "ccad.util.box"

---@class Shape

local M = {}

--- Create a thin-walled rectangular box (open top).
--- Side walls and bottom have uniform thickness `t`.
--- The outer solid starts at the origin.
---@param w number  outer width  (X)
---@param d number  outer depth  (Y)
---@param h number  height       (Z)
---@param t number  wall thickness (uniform)
---@param fillet_radius? number  optional global edge fillet radius
---@return Shape
function M.open_box(w, d, h, t, fillet_radius) end

--- Convenience: centered rectangular solid (XY centered around 0).
---@param w number
---@param d number
---@param h number
---@return Shape
function M.centered(w, d, h) end

--- Planar rounded rectangle face in XY.
---@param w number  width (X)
---@param d number  depth (Y)
---@param r number  corner radius (>=0). Clamped to fit.
---@param seg? integer  arc tessellation per 90° corner (default 16)
---@return Shape  planar face
function M.rounded_rect_face(w, d, r, seg) end

--- Rounded rectangular solid (extruded rounded rectangle).
---@param w number
---@param d number
---@param h number
---@param r number
---@param seg? integer
---@return Shape
function M.rounded_box(w, d, h, r, seg) end

--- Hollow rounded box (open or closed top).
--- Outer rounded box minus inner rounded box with reduced size + radius.
---@param w number  outer width
---@param d number  outer depth
---@param h number  height
---@param t number  wall thickness (uniform)
---@param r number  outer corner radius
---@param seg? integer
---@param open_top? boolean  if true, inner height = h (open)
---@return Shape
function M.rounded_shell(w, d, h, t, r, seg, open_top) end

return M
