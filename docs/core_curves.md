# Curves

Curved modeling tools help you create lathe-like solids from profiles and smooth, doubly-curved plates. These are ideal for bottles, vases, ducts, fairings, and aesthetic panels.

## lathe(opts)

Revolves a profile around the Z-axis to create a solid or hollow body.

!!! info Profile plane

    The control points are given in XZ: x = radius, z = height. This matches the profile_xz convention used by revolve.

### Options (LatheOptions)

- `points`: ({[1]:number,[2]:number})[] — control points in XZ order {x, z}
- `angle?`: number — revolve angle in degrees (default 360)
- `thickness?`: number — wall thickness in mm; if set, a hollow body is created by auto-offsetting and subtracting an inner silhouette
- **Returns** Shape — the revolved solid (watertight)

### Examples

1. Solid, full revolution

```lua
local vase = lathe{
points = {
{0, 0}, -- start on axis for a sealed bottom
{30, 0},
{28, 10},
{20, 60},
{10, 90},
{0, 100}, -- touch axis again for a sealed top
},
angle = 360,
}
emit(vase)
```

2. Hollow shell with 2 mm wall

```lua
local bottle = lathe{
points = {
{0, 0}, {32, 0}, {34, 8}, {30, 40}, {18, 90}, {12, 120}
},
thickness = 2.0, -- creates an inner offset and subtracts it
}
emit(bottle)
```

3. Partial revolve (a 120° segment)

```lua
local fairing_segment = lathe{
points = { {0,0}, {20,0}, {25,15}, {18,40}, {8,60} },
angle = 120,
thickness = 1.2,
}
emit(fairing_segment)
```

## Practical notes

- To obtain a capped (closed) solid with angle=360, make the first and last points lie on the Z-axis (x=0).
- For partial angles, the ends are open like cut sections—cap them via Booleans if needed.
- When using thickness, ensure the inner radius stays positive everywhere: x > thickness.

## curved_plate_xy(size_x, size_y, thickness, k_u, k_v)

Creates a doubly-curved plate by fitting a BSpline surface to a height field z(x,y), offsetting it by ±thickness/2, and closing the sides with ruled faces. The result is a watertight solid suitable for Booleans and export.

### Parameters

- size_x: number — extent along X (mm)
- size_y: number — extent along Y (mm)
- thickness: number — plate thickness (mm)
- k_u: number — BSpline surface control parameter along U/X
- k_v: number — BSpline surface control parameter along V/Y

!!! info About k_u, k_v

    These parameters control the smoothness/complexity of the fitted surface along X and Y (e.g., BSpline order or control density depending on implementation). Use modest integers (e.g., 3–6) for smooth plates; increase if you need more flexibility.

### Examples

1. Gentle saddle panel

```lua
-- 120×80 panel, 3 mm thick, smooth surface
local panel = curved_plate_xy(120, 80, 3, 4, 4)
emit(panel)
```

2. Stiffer curvature (more flexible surface fit)

```lual
local styled_panel = curved_plate_xy(150, 100, 2.5, 6, 5)
emit(styled_panel)
```

Use cases

- Aerodynamic covers, fairings, dashboards
- Decorative/ergonomic panels blending into enclosures
- Subtractive molds: difference the plate from a block to imprint curvature

## Tips & Gotchas

- Units & axes: Distances are in mm; lathe axis is Z.
- Sealed lathes: Start/end points on the Z-axis (x=0) for closed ends.
- Wall checks: With thickness, ensure thickness < min(x) along the profile.
- Partial segments: lathe{ angle < 360 } produces open arcs—cap or union as needed.
- Surface tuning: Increase k_u, k_v for more complex curvature; reduce for simpler, lighter geometry.
- Post-processing: Combine with fillet_all/chamfer_all for print-safe edges or with Booleans for cutouts and mounts.

Curved tools complement sketches and construction: use `lathe` for radially symmetric bodies and `curved_plate_xy` when you need smooth, free-form surfaces that remain solid and CAD-friendly.
