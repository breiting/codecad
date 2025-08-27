---@meta
---@module "ccad.util.bom"

local bom = {}

--- Clear collected items.
function bom.clear() end

--- Return a copy of collected items.
---@return table[]
function bom.items() end

--- Add a BOM row.
---@param t { name:string, qty:integer, material?:string, category?:string,
---          w?:number, d?:number, h?:number, diameter?:number, thickness?:number,
---          notes?:string, visible?:boolean }
function bom.add(t) end

--- Write CSV file (semicolon-separated).
---@param path string
function bom.to_csv(path) end

--- Return Markdown table as string.
---@return string
function bom.to_markdown_string() end

--- Write Markdown table to file.
---@param path string
function bom.to_md(path) end

return bom
