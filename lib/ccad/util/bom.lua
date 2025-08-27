-- lib/ccad/util/bom.lua
--- @module "ccad.util.bom"
-- Minimal BOM/Cutlist collector for project-wide aggregation.
-- You call bom.add{} in each part.lua. The CLI aggregates across parts.

local bom = {}

-- internal store (kept module-local, not global)
local _items = {}

--- Clear items (CLI calls this before executing a part)
function bom.clear()
	_items = {}
end

--- Return a shallow copy of items (for C++ to read)
function bom.items()
	local out = {}
	for i = 1, #_items do
		out[i] = _items[i]
	end
	return out
end

-- helpers ------------------------------------------------------------

local function mm_or_nil(v)
	if v == nil then
		return nil
	end
	assert(type(v) == "number", "dimension must be a number (mm)")
	return v
end

local function str_or_nil(v)
	if v == nil then
		return nil
	end
	assert(type(v) == "string", "field must be a string")
	return v
end

local function bool_or_nil(v)
	if v == nil then
		return nil
	end
	assert(type(v) == "boolean", "field must be boolean")
	return v
end

local function int_or_nil(v)
	if v == nil then
		return nil
	end
	assert(type(v) == "number" and math.type(v) == "integer", "field must be integer")
	return v
end

local function trim(s)
	return (s:gsub("^%s+", ""):gsub("%s+$", ""))
end

-- API ----------------------------------------------------------------

--- Add a BOM / Cutlist row for this part.
--- All lengths are in **mm**. Provide either {w,d,h} (rect. stock) **or** {diameter,h} (round).
--- @param t table
---   t.name        string   -- human label, e.g. "Table Top"
---   t.qty         integer  -- quantity in this part (>=1)
---   t.material?   string   -- e.g. "Spruce", "Oak"
---   t.category?   string   -- e.g. "board", "beam", "round"
---   t.w?          number   -- width (X)  [mm]
---   t.d?          number   -- depth (Y)  [mm]
---   t.h?          number   -- height/length (Z) [mm]
---   t.diameter?   number   -- diameter [mm] (alternative to w/d)
---   t.thickness?  number   -- optional; for boards
---   t.notes?      string
---   t.visible?    boolean  -- default true (for filtering)
function bom.add(t)
	assert(type(t) == "table", "bom.add: table required")
	local name = str_or_nil(t.name) or "Item"
	local qty = int_or_nil(t.qty) or 1
	assert(qty >= 1, "qty must be >= 1")

	local material = t.material and trim(str_or_nil(t.material)) or nil
	local category = t.category and trim(str_or_nil(t.category)) or nil
	local notes = t.notes and trim(str_or_nil(t.notes)) or nil
	local visible = t.visible == nil and true or bool_or_nil(t.visible)

	local w = mm_or_nil(t.w)
	local d = mm_or_nil(t.d)
	local h = mm_or_nil(t.h)
	local diameter = mm_or_nil(t.diameter)
	local thickness = mm_or_nil(t.thickness)

	-- Validate: either rectangular (w/d/h) or round (diameter/h)
	if diameter then
		assert(h ~= nil, "round stock needs 'h' (height/length)")
		-- ignore w/d if present
		w, d = nil, nil
	else
		assert(w ~= nil and d ~= nil and h ~= nil, "rectangular stock needs w,d,h")
	end

	_items[#_items + 1] = {
		name = name,
		qty = qty,
		material = material,
		category = category,
		w = w,
		d = d,
		h = h,
		diameter = diameter,
		thickness = thickness,
		notes = notes,
		visible = visible,
	}
end

-- Simple CSV writer (semicolon-separated to avoid locale comma clashes)
local function to_csv_rows(items)
	local rows = { "name;qty;material;category;w_mm;d_mm;h_mm;diameter_mm;thickness_mm;notes" }
	local function esc(s)
		if s == nil then
			return ""
		end
		s = tostring(s)
		if s:find('[;\n"]') then
			s = '"' .. s:gsub('"', '""') .. '"'
		end
		return s
	end
	for _, it in ipairs(items) do
		if it.visible ~= false then
			table.insert(
				rows,
				table.concat({
					esc(it.name),
					esc(it.qty),
					esc(it.material or ""),
					esc(it.category or ""),
					esc(it.w or ""),
					esc(it.d or ""),
					esc(it.h or ""),
					esc(it.diameter or ""),
					esc(it.thickness or ""),
					esc(it.notes or ""),
				}, ";")
			)
		end
	end
	return table.concat(rows, "\n")
end

--- Write CSV to `path`.
--- @param path string
function bom.to_csv(path)
	local f, err = io.open(path, "wb")
	assert(f, "bom.to_csv: " .. tostring(err))
	f:write(to_csv_rows(_items))
	f:close()
end

--- Generate Markdown table (string).
--- @return string
function bom.to_markdown_string()
	local lines = {}
	local function push(s)
		lines[#lines + 1] = s
	end
	push("| Name | Qty | Material | Category | W (mm) | D (mm) | H (mm) | Ø (mm) | t (mm) | Notes |")
	push("|---:|---:|---|---|---:|---:|---:|---:|---:|---|")
	for _, it in ipairs(_items) do
		if it.visible ~= false then
			push(
				string.format(
					"| %s | %d | %s | %s | %s | %s | %s | %s | %s | %s |",
					it.name or "",
					it.qty or 1,
					it.material or "",
					it.category or "",
					it.w and string.format("%.0f", it.w) or "",
					it.d and string.format("%.0f", it.d) or "",
					it.h and string.format("%.0f", it.h) or "",
					it.diameter and string.format("%.0f", it.diameter) or "",
					it.thickness and string.format("%.0f", it.thickness) or "",
					it.notes or ""
				)
			)
		end
	end
	return table.concat(lines, "\n")
end

--- Write Markdown table to `path`.
--- @param path string
function bom.to_md(path)
	local s = bom.to_markdown_string()
	local f, err = io.open(path, "wb")
	assert(f, "bom.to_md: " .. tostring(err))
	f:write(s)
	f:close()
end

return bom
