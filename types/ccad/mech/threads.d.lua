---@meta
---@module "ccad.mech.threads"

---@class Shape

local M = {}

--- Metric bolt with hex head (very simple head model).
---@param d number         @Nominal diameter (e.g. 6.0 for M6)
---@param length number    @Total length (Z)
---@param opts table?      @{ pitch?, head_h?, thread_len?, xy?, lead_in?, left? }
---@return Shape
function M.bolt_M(d, length, opts) end

--- Metric hex nut with internal thread.
---@param d number         @Nominal diameter (e.g. 6.0 for M6)
---@param height number    @Nut height (Z)
---@param opts table?      @{ pitch?, flat?, xy?, lead_in?, left? }
---@return Shape
function M.nut_M(d, height, opts) end
