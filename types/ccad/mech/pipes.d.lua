---@meta

--- Build a smooth revolved transition between two pipes.
--- Radii are in millimeters; z-axis is the pipe axis; result is a Solid.
---@param rIn0 number       -- inner radius at z=0
---@param rOut0 number      -- outer radius at z=0
---@param rIn1 number       -- inner radius at z=length
---@param rOut1 number      -- outer radius at z=length
---@param length number     -- axial distance (mm)
---@param steepness? number -- smoothing factor between [4..12]
---@return Shape
function pipe_transition(rIn0, rOut0, rIn1, rOut1, length, steepness) end
