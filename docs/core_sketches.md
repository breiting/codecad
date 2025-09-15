# Sketches

Sketches are 2D profiles you can later `extrude` or `revolve` into 3D solids. CodeCAD’s sketch helpers operate on fixed planes for simplicity:

- `poly_xy` and `rect` create profiles on the XY plane (Z = 0).
- `profile_xz` creates profiles on the XZ plane (Y = 0), which is convenient for revolved parts.

Use sketches to describe outlines, ribs, grooves, and lathe-like shapes in a compact, parametric way.

!!! tip

    A closed profile (a loop) can form a face and be extruded into a solid. An open profile is usually for revolve (e.g., a lathe path) or remains an edge curve.

## poly_xy(pts)

Builds a planar polygonal face on the XY plane from a list of points. The polygon is closed by default.

```lua
--- pts: array of 2D points on XY.
--- You can pass {x=..., y=...} or tuple-like {x, y}.
local s = poly_xy({
{0, 0},
{40, 0},
{30, 20},
{10, 25},
{0, 10},
})
```

### Parameters

- **pts**: Points must define a non-self-intersecting loop in either clockwise or counter-clockwise order.
- **Returns**: Shape — a planar face on Z = 0.

### Why use it?

Great for arbitrary brackets, gussets, and plates that you’ll later extrude.

### Example: Extrude a 2D plate

```lua
--8<-- "docs/assets/lua/poly_xy.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/poly_xy.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## profile_xz(pts, closed?)

Builds a polyline on the XZ plane (Y = 0). By default it is open. Set closed=true to close it into a loop (forming a face on XZ).

```lua
--- pts: array of {x,z} or tuple-like {x, z}
--- closed: optional boolean, default false
local p = profile_xz({
{0, 0},
{10, 0},
{10, 20},
{5, 25},
})
```

### Parameters

- pts: ({[1]:number,[2]:number}|{x:number,z:number})[]
- closed?: boolean (default false)
- Returns
  Shape — an edge (open) or a planar face (if closed=true) on Y = 0.

Common use: revolve about Z-axis
Supply an open profile that describes the radius vs. height, then revolve it around the Z-axis.

### Example

```lua
--8<-- "docs/assets/lua/profile_xz.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/profile_xz.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

!!! note

    If you need a capped revolved solid but your curve does not touch the axis, add points that go to x=0 at the ends (i.e., meet the Z-axis) or pass closed=true to make a face first and revolve that.

## rect(w, h)

Draws an axis-aligned rectangle on the XY plane with its lower-left corner at (0,0).

```lua
local r = rect(40, 20) -- width=40, height=20
```

### Parameters

- w: number — width along +X
- h: number — height along +Y
- Returns
  Shape — a rectangular face on Z = 0.

## Point Formats

You can mix either style within one call:

- Tuple-like: {x, y} or {x, z}
- Named fields: {x=..., y=...} or {x=..., z=...}

```lua
local s = poly_xy({
{0,0}, {50,0}, {x=50,y=30}, {x=0,y=30}
})
```

## Practical Patterns

- Holes in plates: Create outer face with `poly_xy`, `extrude`, then difference a cylinder or another `poly_xy/extrude` for cutouts.
- Lathe parts: Draft an open `profile_xz` where x is radius and `revolve`.
- Parametric design: Store dimensions in variables and compute points; small math changes reshape the whole model.

## Gotchas & Tips

- Winding / self-intersections: Ensure polygon points don’t cross; ambiguous loops cannot form faces.
- Planarity: Sketch helpers lock you to XY or XZ; for other orientations, sketch first then rotate\_\* or translate.
- Closing profiles for revolve: An open profile_xz that touches the Z-axis at its ends produces a sealed revolve. Otherwise, add axis-touching points or use closed=true before revolve.

With these three tools - `poly_xy`, `profile_xz`, and `rect` - you can author most 2D outlines needed for mechanical parts, then lift them into 3D using `extrude` or `revolve`.
