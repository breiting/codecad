-- lib/core/component.lua
-- General wrapper around a Shape with transform & properties.
-- Provides: OO-style API, bbox helpers, project (de)serialization.

local C = {}
C.__index = C

local function deepcopy(t)
	if type(t) ~= "table" then
		return t
	end
	local r = {}
	for k, v in pairs(t) do
		r[k] = deepcopy(v)
	end
	return r
end

--- Create a new Component.
--- @param shape Shape
--- @param props table? { id?, name?, material?, color?, tags?[], meta?{}, slot?, explode?vec3,
---                       transform?{translate{xyz}, rotate{xyz_deg}, scale{xyz}}, bed_place?{xy{2},angle} }
function C.new(shape, props)
	local self = setmetatable({}, C)
	self._shape = shape
	self.id = props and props.id or nil
	self.name = props and props.name or "component"
	self.material = props and props.material or nil
	self.color = props and props.color or nil
	self.tags = (props and props.tags) and deepcopy(props.tags) or {}
	self.meta = (props and props.meta) and deepcopy(props.meta) or {}

	-- viewer/assembly hints
	self.slot = props and props.slot or "default"
	self.explode = props and props.explode or { 0, 0, 0 }
	self.bed_place = props and props.bed_place or nil

	-- transform (local)
	local t = (props and props.transform) or {}
	self.t = {
		translate = t.translate or { 0, 0, 0 },
		rotate = t.rotate or { 0, 0, 0 }, -- degrees about X,Y,Z
		scale = t.scale or { 1, 1, 1 },
	}
	return self
end

-- ===== transforms (fluent) =====

function C:Move(dx, dy, dz)
	local v = self.t.translate
	v[1] = v[1] + (dx or 0)
	v[2] = v[2] + (dy or 0)
	v[3] = v[3] + (dz or 0)
	return self
end

function C:SetPosition(x, y, z)
	self.t.translate = { x or 0, y or 0, z or 0 }
	return self
end

function C:Rotate(rx, ry, rz) -- degrees, additive
	local r = self.t.rotate
	r[1] = r[1] + (rx or 0)
	r[2] = r[2] + (ry or 0)
	r[3] = r[3] + (rz or 0)
	return self
end

function C:SetRotation(rx, ry, rz)
	self.t.rotate = { rx or 0, ry or 0, rz or 0 }
	return self
end

function C:Scale(sx, sy, sz)
	local s = self.t.scale
	s[1] = s[1] * (sx or 1)
	s[2] = s[2] * (sy or 1)
	s[3] = s[3] * (sz or 1)
	return self
end

function C:SetScale(sx, sy, sz)
	self.t.scale = { sx or 1, sy or 1, sz or 1 }
	return self
end

-- ===== computed shape =====

--- Get world-shape (shape with this component's transform applied)
function C:Shape()
	local s = self._shape
	local sc = self.t.scale
	if sc[1] ~= 1 or sc[2] ~= 1 or sc[3] ~= 1 then
		s = scale_xyz(s, sc[1], sc[2], sc[3])
	end
	local r = self.t.rotate
	if r[1] ~= 0 then
		s = rotate_x(s, r[1])
	end
	if r[2] ~= 0 then
		s = rotate_y(s, r[2])
	end
	if r[3] ~= 0 then
		s = rotate_z(s, r[3])
	end
	local tr = self.t.translate
	if tr[1] ~= 0 or tr[2] ~= 0 or tr[3] ~= 0 then
		s = translate(s, tr[1], tr[2], tr[3])
	end
	return s
end

-- ===== bbox & anchors =====

local function bb_of(shape)
	local b = bbox(shape, true)
	return {
		minX = b.min.x,
		minY = b.min.y,
		minZ = b.min.z,
		maxX = b.max.x,
		maxY = b.max.y,
		maxZ = b.max.z,
		sizeX = b.size.x,
		sizeY = b.size.y,
		sizeZ = b.size.z,
		cx = b.center.x,
		cy = b.center.y,
		cz = b.center.z,
	}
end

function C:BBox()
	return bb_of(self:Shape())
end

function C:MinX()
	return self:BBox().minX
end

function C:MaxX()
	return self:BBox().maxX
end

function C:MinY()
	return self:BBox().minY
end

function C:MaxY()
	return self:BBox().maxY
end

function C:MinZ()
	return self:BBox().minZ
end

function C:MaxZ()
	return self:BBox().maxZ
end

function C:CenterX()
	return self:BBox().cx
end

function C:CenterY()
	return self:BBox().cy
end

function C:CenterZ()
	return self:BBox().cz
end

function C:SizeX()
	return self:BBox().sizeX
end

function C:SizeY()
	return self:BBox().sizeY
end

function C:SizeZ()
	return self:BBox().sizeZ
end

-- ===== project (de)serialization =====

--- Convert to project-part table (JSON-ready)
function C:ToProjectPart(fields)
	local t = {
		name = self.name,
		slot = self.slot,
		material = self.material,
		color = self.color,
		transform = {
			translate = { self.t.translate[1], self.t.translate[2], self.t.translate[3] },
			rotate = { self.t.rotate[1], self.t.rotate[2], self.t.rotate[3] },
			scale = { self.t.scale[1], self.t.scale[2], self.t.scale[3] },
		},
		explode = { self.explode[1], self.explode[2], self.explode[3] },
		bed_place = self.bed_place and {
			xy = { self.bed_place.xy[1], self.bed_place.xy[2] },
			angle = self.bed_place.angle or 0,
		} or nil,
		tags = self.tags,
		meta = self.meta,
	}
	-- optional fields passthrough (e.g., stl path)
	if fields and fields.stl then
		t.stl = fields.stl
	end
	if fields and fields.source then
		t.source = fields.source
	end
	return t
end

--- Construct from a project-part entry and a shape-provider.
--- @param entry table  (project parts[i])
--- @param get_shape fun(entry:table):Shape  -- e.g., load STL or build from Lua
function C.from_project(entry, get_shape)
	local shape = get_shape(entry)
	return C.new(shape, {
		id = entry.id,
		name = entry.name,
		material = entry.material,
		color = entry.color,
		tags = entry.tags,
		meta = entry.meta,
		slot = entry.slot,
		explode = entry.explode or { 0, 0, 0 },
		bed_place = entry.bed_place,
		transform = entry.transform,
	})
end

return C
