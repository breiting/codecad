-- util/frame.lua
-- A tiny transform stack for "virtual origins" and local frames.
-- Apply current frame to shapes, or wrap primitives so they place themselves.

local M = {}

-- internal frame: position (tx,ty,tz), rotation (rx,ry,rz, degrees), uniform scale (sc)
local _stack = { { tx = 0, ty = 0, tz = 0, rx = 0, ry = 0, rz = 0, sc = 1 } }

local function _top()
	return _stack[#_stack]
end

--- Push a copy of the current frame onto the stack.
function M.push()
	local t = _top()
	_stack[#_stack + 1] = { tx = t.tx, ty = t.ty, tz = t.tz, rx = t.rx, ry = t.ry, rz = t.rz, sc = t.sc }
end

--- Pop the current frame. (Keeps at least one frame.)
function M.pop()
	if #_stack > 1 then
		_stack[#_stack] = nil
	end
end

--- Temporarily enter a new scope (push → fn() → pop).
function M.scope(fn)
	M.push()
	local ok, err = pcall(fn)
	M.pop()
	if not ok then
		error(err, 0)
	end
end

--- Translate current frame by (dx,dy,dz).
function M.origin(dx, dy, dz)
	local t = _top()
	t.tx = t.tx + (dx or 0)
	t.ty = t.ty + (dy or 0)
	t.tz = t.tz + (dz or 0)
end

--- Rotate current frame (adds degrees).
function M.rotate_x(deg)
	local t = _top()
	t.rx = t.rx + (deg or 0)
end

function M.rotate_y(deg)
	local t = _top()
	t.ry = t.ry + (deg or 0)
end

function M.rotate_z(deg)
	local t = _top()
	t.rz = t.rz + (deg or 0)
end

--- Uniform scale for the current frame (multiplies).
function M.scale(f)
	local t = _top()
	t.sc = t.sc * (f or 1)
end

--- Apply current frame to a shape and return the transformed shape.
--  Order: scale → rotateX → rotateY → rotateZ → translate.
function M.apply(shape)
	local t = _top()
	local s = shape
	if t.sc ~= 1 then
		s = scale(s, t.sc)
	end
	if t.rx ~= 0 then
		s = rotate_x(s, t.rx)
	end
	if t.ry ~= 0 then
		s = rotate_y(s, t.ry)
	end
	if t.rz ~= 0 then
		s = rotate_z(s, t.rz)
	end
	if t.tx ~= 0 or t.ty ~= 0 or t.tz ~= 0 then
		s = translate(s, t.tx, t.ty, t.tz)
	end
	return s
end

--- Wrap core primitives to auto-apply the current frame.
--  Usage:
--    local F = require("util.frame")
--    local P = F.wrap_primitives()
--    local b = P.box(10,10,10)   -- already placed in current frame
function M.wrap_primitives()
	local P = {}
	function P.box(x, y, z)
		return M.apply(box(x, y, z))
	end

	function P.cylinder(d, h)
		return M.apply(cylinder(d, h))
	end

	function P.hex_prism(af, h)
		return M.apply(hex_prism(af, h))
	end

	return P
end

--- Sugar: with_origin(dx,dy,dz, fn) – runs fn in a shifted local frame.
function M.with_origin(dx, dy, dz, fn)
	M.scope(function()
		M.origin(dx, dy, dz)
		fn()
	end)
end

--- Sugar: with_frame({tx,ty,tz, rx,ry,rz, sc}, fn)
function M.with_frame(fr, fn)
	M.scope(function()
		if fr.sc then
			M.scale(fr.sc)
		end
		if fr.rx then
			M.rotate_x(fr.rx)
		end
		if fr.ry then
			M.rotate_y(fr.ry)
		end
		if fr.rz then
			M.rotate_z(fr.rz)
		end
		if fr.tx or fr.ty or fr.tz then
			M.origin(fr.tx or 0, fr.ty or 0, fr.tz or 0)
		end
		fn()
	end)
end

return M
