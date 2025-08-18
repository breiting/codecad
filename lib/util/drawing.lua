-- util/drawing.lua : convenience helpers around section_outline + svg
local svg = require("util.svg")

local D = {}

--- Create a top-view SVG outline by cutting shape at z (default: minZ).
--- scale: drawing mm per model mm (1.0 = 1:1 on SVG mm).
function D.top_view_svg(shape, path, opts)
	opts = opts or {}
	local b = bbox(shape, true)
	local z = opts.z or b.min.z
	local scale = opts.scale or 1.0
	local margin = opts.margin or 10

	-- section
	local polys = section_outline(shape, "z", z, opts.defl or 0.2)

	-- compute extents of outline
	local minx, miny, maxx, maxy = 1e9, 1e9, -1e9, -1e9
	for _, pl in ipairs(polys) do
		for _, p in ipairs(pl) do
			if p[1] < minx then
				minx = p[1]
			end
			if p[2] < miny then
				miny = p[2]
			end
			if p[1] > maxx then
				maxx = p[1]
			end
			if p[2] > maxy then
				maxy = p[2]
			end
		end
	end
	if minx > maxx or miny > maxy then
		error("top_view_svg: empty outline (section produced no edges)", 0)
	end

	local W = (maxx - minx) * scale + 2 * margin
	local H = (maxy - miny) * scale + 2 * margin
	local doc = svg.begin(W, H, { bg = opts.bg })

	-- shift & scale points
	local styled = {}
	for i, pl in ipairs(polys) do
		local q = {}
		for j, p in ipairs(pl) do
			q[j] = { (p[1] - minx) * scale + margin, (p[2] - miny) * scale + margin }
		end
		styled[i] = q
	end

	svg.polylines(doc, styled, opts.stroke or "stroke:#000;stroke-width:0.35")
	-- optional dims
	if opts.dim_overall then
		local dx = (maxx - minx) * scale
		local dy = (maxy - miny) * scale
		svg.dim(doc, margin, margin - 3, margin + dx, margin - 3, string.format("W = %.0f", (maxx - minx)), 6)
		svg.dim(doc, margin - 3, margin, margin - 3, margin + dy, string.format("D = %.0f", (maxy - miny)), 6)
	end

	svg.save(doc, path)
end

return D
