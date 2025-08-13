local M = {}

--- Generates an open box with a certain size and wall thickness
--- @param L number @length (mm)
--- @param W number @width (mm)
--- @param H number @height (mm)
--- @param t number @wall thickness (mm)
function M.open_box(L, W, H, t, edge)
	local outer = box(L, W, H)
	local inner = translate(box(L - 2 * t, W - 2 * t, H - t), t, t, t)
	local r = difference(outer, inner)
	if edge and edge > 0 then
		return fillet(r, edge)
	end
	return r
end

return M
