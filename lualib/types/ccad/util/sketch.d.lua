---@meta
---@module "ccad.util.sketch"

---@class Shape

---@class ccad.util.sketch.Sketch
local Sketch = {}

--- Begin a sketch at (x, y). Defaults to (0,0).
---@param x? number
---@param y? number
---@return ccad.util.sketch.Sketch
function Sketch.begin(x, y) end

--- Move to absolute (x, y), starting a fresh path (single-loop model).
---@param x number
---@param y number
---@return ccad.util.sketch.Sketch
function Sketch:move_to(x, y) end

--- Line to absolute (x, y).
---@param x number
---@param y number
---@return ccad.util.sketch.Sketch
function Sketch:line_to(x, y) end

--- Circular arc **clockwise** around center (cx, cy) with radius r,
--- sweeping by `sweep_deg` degrees.
---@param cx number  center X
---@param cy number  center Y
---@param r number   radius (>0)
---@param sweep_deg number  CW sweep angle in degrees
---@param seg? integer  tessellation segments (default 16, min 4)
---@return ccad.util.sketch.Sketch
function Sketch:arc_cw(cx, cy, r, sweep_deg, seg) end

--- Circular arc **counter-clockwise** around center (cx, cy) with radius r,
--- sweeping by `sweep_deg` degrees.
---@param cx number  center X
---@param cy number  center Y
---@param r number   radius (>0)
---@param sweep_deg number  CCW sweep angle in degrees
---@param seg? integer  tessellation segments (default 16, min 4)
---@return ccad.util.sketch.Sketch
function Sketch:arc_ccw(cx, cy, r, sweep_deg, seg) end

--- Close the path back to the first point.
---@return ccad.util.sketch.Sketch
function Sketch:close() end

--- Build a planar face in the XY plane using the accumulated polyline.
---@return Shape
function Sketch:face() end

return Sketch
