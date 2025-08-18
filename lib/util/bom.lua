-- util/bom.lua
local M = { items = {} }

local function key(it)
	return table.concat({ it.name or "item", it.material or "", it.dim or "", it.note or "" }, "|")
end

function M.reset()
	M.items = {}
end

--- Add one item (auto-aggregate by (name, material, dim, note)).
-- it = { name, qty, material?, dim?, note? }
function M.add(it)
	it.qty = it.qty or 1
	local k = key(it)
	local e = M.items[k]
	if e then
		e.qty = e.qty + it.qty
	else
		M.items[k] = { name = it.name, qty = it.qty, material = it.material, dim = it.dim, note = it.note }
	end
end

function M.to_array()
	local arr = {}
	for _, v in pairs(M.items) do
		arr[#arr + 1] = v
	end
	table.sort(arr, function(a, b)
		return a.name < b.name
	end)
	return arr
end

function M.save_csv(path)
	local f = assert(io.open(path, "w"))
	f:write("Name;Qty;Material;Dimensions;Note\n")
	for _, it in ipairs(M.to_array()) do
		f:write(("%s;%d;%s;%s;%s\n"):format(it.name, it.qty, it.material or "", it.dim or "", it.note or ""))
	end
	f:close()
end

function M.save_md(path)
	local f = assert(io.open(path, "w"))
	f:write("| Name | Qty | Material | Dimensions | Note |\n|---:|---:|---|---|---|\n")
	for _, it in ipairs(M.to_array()) do
		f:write(
			("| %s | %d | %s | %s | %s |\n"):format(it.name, it.qty, it.material or "", it.dim or "", it.note or "")
		)
	end
	f:close()
end

function M.save_json(path)
	local f = assert(io.open(path, "w"))
	local first = true
	f:write('{"items":[')
	for _, it in ipairs(M.to_array()) do
		if not first then
			f:write(",")
		end
		first = false
		f:write(
			string.format(
				'{"name":"%s","qty":%d,"material":"%s","dim":"%s","note":"%s"}',
				it.name,
				it.qty,
				it.material or "",
				it.dim or "",
				it.note or ""
			)
		)
	end
	f:write("]}")
	f:close()
end

return M
