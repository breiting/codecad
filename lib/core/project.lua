-- core/project.lua
-- Build & save a CodeCAD project JSON from a list of Components.

local Project = {}

-- --- tiny JSON encoder (suffices for our project structures)
local function esc(s)
	return (
		s:gsub('[\\"\b\f\n\r\t]', {
			["\\"] = "\\\\",
			['"'] = '\\"',
			["\b"] = "\\b",
			["\f"] = "\\f",
			["\n"] = "\\n",
			["\r"] = "\\r",
			["\t"] = "\\t",
		})
	)
end
local function encode(v)
	local tv = type(v)
	if tv == "nil" then
		return "null"
	elseif tv == "boolean" then
		return v and "true" or "false"
	elseif tv == "number" then
		if v ~= v or v == math.huge or v == -math.huge then
			return "null"
		end
		return tostring(v)
	elseif tv == "string" then
		return '"' .. esc(v) .. '"'
	elseif tv == "table" then
		-- detect array
		local n, arr = 0, true
		for k, _ in pairs(v) do
			if type(k) ~= "number" then
				arr = false
				break
			end
			if k > n then
				n = k
			end
		end
		if arr and n == #v then
			local parts = {}
			for i = 1, #v do
				parts[i] = encode(v[i])
			end
			return "[" .. table.concat(parts, ",") .. "]"
		else
			local parts = {}
			for k, val in pairs(v) do
				parts[#parts + 1] = encode(tostring(k)) .. ":" .. encode(val)
			end
			return "{" .. table.concat(parts, ",") .. "}"
		end
	else
		return "null"
	end
end

--- Build a project table from Components.
--- @param components Component[]
--- @param meta table|nil  e.g. { name="My Project", author="me", units="mm" }
--- @return table project
function Project.build(components, meta)
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

--- Save a project file (JSON) from Components.
--- @param components Component[]
--- @param path string
--- @param meta table|nil
function Project.save_project(components, path, meta)
	local doc = Project.build(components, meta)
	local json = encode(doc)
	local f = assert(io.open(path, "w"))
	f:write(json)
	f:close()
end

return Project
