---@meta
---@module "ccad.util.shape2d"

---@class Shape

local S2 = {}

--- Axis-aligned rectangle face with lower-left at (0,0).
---@param w number
---@param d number
---@return Shape
function S2.rect(w, d) end

--- Circle face centered at (0,0) with diameter d.
---@param d number
---@param seg? integer
---@return Shape
function S2.circle(d, seg) end

--- Annulus (ring) as a single face (outer CCW, inner CW).
---@param d_o number outer diameter
---@param d_i number inner diameter
---@param seg? integer
---@return Shape
function S2.ring(d_o, d_i, seg) end

--- Rounded slot (obround) aligned along X: length w, height h.
---@param w number overall length
---@param h number overall height
---@param seg? integer arc tessellation
---@return Shape
function S2.slot_xy(w, h, seg) end

--- “D-profile” face: rectangle base (0..w, 0..h_rect) + top semicircle.
---@param w number
---@param h_rect number
---@param r_top number
---@param seg? integer
---@return Shape
function S2.d_profile_face(w, h_rect, r_top, seg) end

return S2
