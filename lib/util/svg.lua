-- util/svg.lua : tiny SVG helper (mm units). Supports polylines and linear dimensions.

local M = {}

local function esc(s)
	return (tostring(s):gsub('[&<>"]', { ["&"] = "&amp;", [">"] = "&gt;", ["<"] = "&lt;", ['"'] = "&quot;" }))
end

function M.begin(width_mm, height_mm, opts)
	opts = opts or {}
	local s = {}
	s.w, s.h = width_mm, height_mm
	s.buf = {}
	local view = string.format('viewBox="0 0 %.3f %.3f"', width_mm, height_mm)
	table.insert(
		s.buf,
		string.format(
			'<svg xmlns="http://www.w3.org/2000/svg" width="%.3fmm" height="%.3fmm" %s>',
			width_mm,
			height_mm,
			view
		)
	)
	if opts.bg then
		table.insert(s.buf, string.format('<rect x="0" y="0" width="100%%" height="100%%" fill="%s"/>', esc(opts.bg)))
	end
	return s
end

function M.finish(s)
	table.insert(s.buf, "</svg>")
	return table.concat(s.buf, "\n")
end

function M.save(s, path)
	local f = assert(io.open(path, "w"))
	f:write(M.finish(s))
	f:close()
end

-- Draw a polyline (pts: { {x,y}, ... }), with style string
function M.polyline(s, pts, style)
	local parts = {}
	for i, p in ipairs(pts) do
		parts[i] = string.format("%.3f,%.3f", p[1], s.h - p[2])
	end -- flip Y for drawing
	table.insert(
		s.buf,
		string.format(
			'<polyline points="%s" style="%s" fill="none"/>',
			table.concat(parts, " "),
			style or "stroke:#000;stroke-width:0.35"
		)
	)
end

-- Draw multiple polylines
function M.polylines(s, arr, style)
	for _, pl in ipairs(arr) do
		M.polyline(s, pl, style)
	end
end

-- Linear dimension with arrows & text (simple)
function M.dim(s, x1, y1, x2, y2, txt, off, style)
	off = off or 6
	style = style or "stroke:#333;stroke-width:0.25"
	-- offset line (parallel)
	local dx, dy = x2 - x1, y2 - y1
	local len = math.sqrt(dx * dx + dy * dy)
	local nx, ny = -dy / len, dx / len -- normal
	local ox, oy = nx * off, ny * off
	local ax1, ay1 = x1 + ox, y1 + oy
	local ax2, ay2 = x2 + ox, y2 + oy

	local function Y(y)
		return s.h - y
	end
	local function L(x1, y1, x2, y2, st)
		table.insert(
			s.buf,
			string.format('<line x1="%.3f" y1="%.3f" x2="%.3f" y2="%.3f" style="%s"/>', x1, Y(y1), x2, Y(y2), st)
		)
	end
	local st = style

	-- dim line
	L(ax1, ay1, ax2, ay2, st)
	-- extension lines
	L(x1, y1, ax1, ay1, st)
	L(x2, y2, ax2, ay2, st)
	-- arrows (simple V)
	local ah = 2.5
	local tx, ty = dx / len, dy / len
	local function arrow(x, y, dir)
		local sx, sy = -tx * dir * ah, -ty * dir * ah
		local px, py = -ny * ah * 0.6, nx * ah * 0.6
		M.polyline(s, { { x, y }, { x + sx + px, y + sy + py } }, st)
		M.polyline(s, { { x, y }, { x + sx - px, y + sy - py } }, st)
	end
	arrow(ax1, ay1, -1)
	arrow(ax2, ay2, 1)

	-- text (middle)
	local mx, my = (ax1 + ax2) / 2, (ay1 + ay2) / 2
	table.insert(
		s.buf,
		string.format(
			'<text x="%.3f" y="%.3f" font-size="3.5" fill="#111" text-anchor="middle" dominant-baseline="central">%s</text>',
			mx,
			Y(my),
			esc(txt or "")
		)
	)
end

return M
