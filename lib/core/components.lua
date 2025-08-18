-- lib/core/components.lua
local Component = require("core.component")
local M = {}

function M.union(components)
	local shapes = {}
	for i, c in ipairs(components) do
		shapes[i] = c:Shape()
	end
	return union(table.unpack(shapes))
end

--- Build a project table from components.
function M.to_project(components, meta)
	local parts = {}
	for _, c in ipairs(components) do
		parts[#parts + 1] = c:ToProjectPart({})
	end
	return {
		version = 1,
		meta = meta or { name = "CodeCAD Project", units = "mm" },
		parts = parts,
	}
end

return M
