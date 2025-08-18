local G = {}

--- Spur gear with sensible defaults.
function G.spur(args)
	local z = args.z or 24
	local m = args.m or 2.5
	local th = args.th or (3 * m)
	local bore = args.bore or math.max(0, 0.4 * z) -- simple default
	local pdeg = args.pressure or 20
	return gear_involute(z, m, th, bore, pdeg)
end

return G
