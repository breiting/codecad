# Construction

Construction operations turn 2D sketches into 3D solids. They are the key bridge between flat outlines and real, machinable geometry. CodeCAD supports two fundamental construction methods: `extrusion` and `revolve`.

## extrude(face, height)

Creates a 3D solid by linearly extruding a planar face/profile along the +Z axis.

```lua
-- Simple plate extruded from a rectangle
local plate2D = rect(80, 40)
local plate3D = extrude(plate2D, 6)
emit(plate3D)
```

### Parameters

- face: Shape — a planar 2D face (e.g. from rect, poly_xy)
- height: number — extrusion height in mm
- **Returns** Shape — solid with the given thickness

### Notes

- Extrusion always extends upwards (+Z) from the sketch plane.
- For downward or symmetric extrusions, combine with translate or center_z.

```lua
-- Symmetric extrusion (thickness 10, centered on Z=0)
local slot = center_xy(rect(40, 10))
slot = center_z(extrude(slot, 10))
emit(slot)
```

## revolve(profile, angle_deg)

Creates a 3D solid by revolving a profile drawn in the XZ plane around the Z-axis at the origin.

```lua
-- Bottle-like shape by revolving an open profile
local outline = profile_xz({
{0, 0}, {30, 0}, {28, 10}, {20, 60}, {0, 60}
})

local body = revolve(outline, 360) -- full revolution
emit(body)
```

### Parameters

- profile: Shape — a profile curve or face in XZ-plane (use profile_xz)
- angle_deg: number — sweep angle in degrees (0–360)
- **Returns** Shape — revolved solid

### Notes

- For open profiles, the curve is revolved as a surface swept around Z.
- To get a closed solid, ensure the profile touches the Z-axis at both ends or use a closed face.
- angle_deg < 360° creates partial revolutions (e.g., 180° = half shell).

```lua
-- Partial revolve: a 90° section of a disk
local arc_profile = profile_xz({{0,0},{30,0},{30,5},{0,5}}, true)
emit(revolve(arc_profile, 90))
```

## Practical Patterns

- Plates, brackets, ribs → extrude a 2D sketch
- Turned parts (shafts, bottles, vases) → revolve a profile
- Symmetric solids → use `center_z` after extrusion, or revolve a symmetric profile
- Combine with Booleans → cut holes, add bosses, and shape parts

## Tips & Gotchas

- Input must be planar: `extrude` and `revolve` require flat profiles (`rect`, `poly_xy`, `profile_xz`).
- Direction: Extrude is fixed along +Z.
- Closed vs open profiles:
  - `extrude` requires closed loops.
  - `revolve` accepts both open (lathe-like) and closed (disk-like) profiles.
- Partial revolves: Great for rings, sections, or curved supports.

Extrusion and revolve are the workhorses of CAD: almost any mechanical part starts from these two construction methods. Use them to lift sketches into solids, then refine with Booleans, transformations, and features.
