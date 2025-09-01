--- @module "ccad.mech.threads"
-- Convenience helpers for metric fasteners (bolts & nuts)
-- Requires core functions: iso_coarse_pitch, metric_thread_external, metric_thread_internal
-- and primitives: hex_prism, cylinder, union, difference, translate, fillet, chamfer (optional)

local M = {}

--- Build a simple hex head for a metric bolt (across flats ~ 1.5*d)
local function hex_head(d_nominal, head_height)
	local across = 1.5 * d_nominal
	return hex_prism(across, head_height)
end

--- Simple shank (unthreaded) as cylinder
local function shank(d_nominal, length)
	return cylinder(d_nominal, length)
end

--- Metric bolt (external thread) with hex head
--- @param d number    Nominal diameter (e.g. 6.0 for M6)
--- @param length number  Total length (Z). Threaded part will be length - head_height unless overridden.
--- @param opts table? { pitch?, head_h?, thread_len?, xy?, lead_in?, left? }
--- @return Shape
function M.bolt_M(d, length, opts)
	opts = opts or {}
	local pitch = opts.pitch or iso_coarse_pitch(d)
	local head_h = opts.head_h or (0.7 * d) -- rough ISO-ish head height
	local thread_len = opts.thread_len or (length - head_h)
	if thread_len < 0 then
		thread_len = 0
	end

	local t_opts = {
		xy = opts.xy or 0.15,
		lead_in = opts.lead_in or 1.0,
		left = opts.left or false,
	}

	local head = hex_head(d, head_h)
	local body = translate(metric_thread_external(d, pitch, thread_len, t_opts), 0, 0, 0)
	local bolt = union(translate(head, 0, 0, thread_len), body)

	-- Optional small fillet on head edges (if available)
	-- bolt = fillet(bolt, 0.2)

	return bolt
end

--- Metric nut (internal thread) as hex prism with threaded bore
--- @param d number   Nominal diameter (e.g. 6.0 for M6)
--- @param height number Nut height (Z)
--- @param opts table? { pitch?, flat?, xy?, lead_in?, left? }
--- @return Shape
function M.nut_M(d, height, opts)
	opts = opts or {}
	local pitch = opts.pitch or iso_coarse_pitch(d)
	local flat = opts.flat or (1.5 * d) -- across flats

	local t_opts = {
		xy = opts.xy or 0.15,
		lead_in = opts.lead_in or 0.8,
		left = opts.left or false,
	}

	local shell = hex_prism(flat, height)
	local core = metric_thread_internal(d, pitch, height, t_opts)
	local nut = difference(shell, core)

	-- Optional chamfer on top/bottom
	-- nut = chamfer(nut, 0.2)

	return nut
end

return M
