local M = {}

function M.rect(w, h)
	return poly_xy({ { 0, 0 }, { w, 0 }, { w, h }, { 0, h } }, true)
end

function M.ring(outer_d, inner_d)
	local o = cylinder(outer_d, 1)
	local i = translate(cylinder(inner_d, 1.2), 0, 0, -0.1)
	return difference(o, i)
end

--- Simple slot (rounded ends) in XY via difference helper
function M.slot_xy(w, h, t)
	local core = translate(box(w - t, h, 1.2), 0, 0, -0.1)
	local caps =
		union(translate(cylinder(t, 1.2), -(w - t) / 2, 0, -0.1), translate(cylinder(t, 1.2), (w - t) / 2, 0, -0.1))
	return union(core, caps)
end

return M
