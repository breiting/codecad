---@class Component
---@field id string|nil
---@field name string
---@field material string|nil
---@field color string|nil
---@field tags string[]
---@field meta table
---@field slot string
---@field explode number[]  -- {x,y,z}
---@field bed_place {xy:number[], angle:number}|nil
---@field t {translate:number[], rotate:number[], scale:number[]}
local Component = {}

---@param shape Shape
---@param props table|nil
function Component.new(shape, props) end

---@param dx string
---@param dy  string
---@param dz string
function Component:Move(dx, dy, dz) end

function Component:SetPosition(x, y, z) end

function Component:Rotate(rx, ry, rz) end

function Component:SetRotation(rx, ry, rz) end

function Component:Scale(sx, sy, sz) end

function Component:SetScale(sx, sy, sz) end

function Component:Shape() end

function Component:BBox() end

function Component:ToProjectPart(fields) end
