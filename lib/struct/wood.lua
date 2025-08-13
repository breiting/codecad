local M = {}

function M.board(L, B, T, edge)
	local s = box(L, B, T)
	if edge and edge > 0 then
		s = chamfer(s, math.min(edge, 0.45 * T))
	end
	return s
end

function M.beam(LX, BY, HZ, edge)
	local s = box(LX, BY, HZ)
	if edge and edge > 0 then
		s = chamfer(s, math.min(edge, 0.45 * math.min(BY, HZ)))
	end
	return s
end

function M.post(S, H, edge)
	local s = box(S, S, H)
	if edge and edge > 0 then
		s = chamfer(s, math.min(edge, 0.45 * S))
	end
	return s
end

return M
