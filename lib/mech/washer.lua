-- Scheibe nach ISO 7089 grob
-- id: Innendurchmesser, od: Außendurchmesser, t: Dicke
local function washer_iso(id, od, t)
	local outer = cylinder(od, t)
	local inner = cylinder(id, t + 0.2)
	inner = translate(inner, 0, 0, -0.1)
	local w = difference(outer, inner)
	w = chamfer(w, math.min(0.2, t * 0.25))
	return w
end

return washer_iso
