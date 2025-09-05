---@meta
---@module "ccad.util.transform"

---@class Shape

local T = {}

--- Translate along X by dx [mm].
---@param shape Shape
---@param dx number
---@return Shape
function T.move_x(shape, dx) end

--- Translate along Y by dy [mm].
---@param shape Shape
---@param dy number
---@return Shape
function T.move_y(shape, dy) end

--- Translate along Z by dz [mm].
---@param shape Shape
---@param dz number
---@return Shape
function T.move_z(shape, dz) end

--- Translate by (dx, dy, dz) [mm].
---@param shape Shape
---@param dx number
---@param dy number
---@param dz number
---@return Shape
function T.move(shape, dx, dy, dz) end

--- Rotate around world X-axis by angle (deg).
---@param shape Shape
---@param angle number
---@return Shape
function T.rot_x(shape, angle) end

--- Rotate around world Y-axis by angle (deg).
---@param shape Shape
---@param angle number
---@return Shape
function T.rot_y(shape, angle) end

--- Rotate around world Z-axis by angle (deg).
---@param shape Shape
---@param angle number
---@return Shape
function T.rot_z(shape, angle) end

--- Rotate with Euler angles, applied in `order` (default "xyz").
--- Valid orders: "xyz","xzy","yxz","yzx","zxy","zyx".
---@param shape Shape
---@param rx number rotation about X (deg)
---@param ry number rotation about Y (deg)
---@param rz number rotation about Z (deg)
---@param order? '"xyz"'|'"xzy"'|'"yxz"'|'"yzx"'|'"zxy"'|'"zyx"'
---@return Shape
function T.rot_euler(shape, rx, ry, rz, order) end

--- Center on X (translates by half width).
---@param shape Shape
---@return Shape
function T.center_x(shape) end

--- Center on Y.
---@param shape Shape
---@return Shape
function T.center_y(shape) end

--- Center on Z.
---@param shape Shape
---@return Shape
function T.center_z(shape) end

--- Center on X and Y.
---@param shape Shape
---@return Shape
function T.center_xy(shape) end

--- Center on X, Y and Z.
---@param shape Shape
---@return Shape
function T.center_xyz(shape) end

--- Translate so the bounding-box center goes to (cx,cy,cz).
---@param shape Shape
---@param cx number
---@param cy number
---@param cz number
---@return Shape
function T.center_to(shape, cx, cy, cz) end

return T
