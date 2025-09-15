# Transformations

Transformations let you move, rotate, and resize shapes in 3D space. They are essential for positioning parts, assembling sub-shapes, and creating symmetric patterns. In CodeCAD, transformations always act in world coordinates (global X/Y/Z).

!!! info

    Every transformation creates a new shape. To update the original, assign the result back to the same variable.

## translate(s, dx, dy, dz)

Moves a shape by the given offsets.

```lua
-- Two cylinders side by side
local c1 = cylinder(5, 20)
local c2 = translate(c1, 15, 0, 0)
emit(union(c1, c2))
```

### Parameters

- s: Shape
- dx: number — offset along X
- dy: number — offset along Y
- dz: number — offset along Z
- **Returns** Shape — the translated shape.

## rotate_x(s, deg)

Rotates a shape around the X-axis, following the right-hand rule.

```lua
local rod = cylinder(3, 40)
emit(rotate_x(rod, 90)) -- lay rod flat along Y
```

### Parameters

- s: Shape
- deg: number — rotation angle in degrees

## rotate_y(s, deg)

Rotates a shape around the Y-axis.

```lua
local boxy = box(20, 10, 5)
emit(rotate_y(boxy, 45))
```

## rotate_z(s, deg)

Rotates a shape around the Z-axis.

```lua
local arm = extrude(rect(60, 10), 5)
emit(rotate_z(arm, 30))
```

## scale(s, factor)

Uniformly scales a shape around the world origin (0,0,0).

```lua
local s = sphere(10)
emit(scale(s, 1.5)) -- 50% larger
```

### Parameters

- s: Shape
- factor: number — scale multiplier
  (1.0 = no change, 2.0 = double size, 0.5 = half size)

!!! note

    Scaling happens around the origin. To scale around a shape’s centroid, use `center_xy`, `center_xyz`, etc., before scaling, then translate back if needed.

## Transformation Helpers

Centering transforms make it easy to align parts around the world origin or move a part’s centroid to a specific location. They work by translating the shape based on its bounding box and center of mass.

### Why this matters

- Rotations and uniform scale behave predictably when the part is centered.
- Symmetric assemblies are simpler when sub-parts sit around (0,0,0).

## center_x(s)

Centers a shape along X around 0 (shifts by half its width).

```lua
local beam = box(80, 20, 10)
emit(center_x(beam))  -- now spans from -40 to +40 along X
```

## center_y(s)

Centers a shape along Y around 0 (shifts by half its depth).

```lua
local rail = box(10, 60, 10)
emit(center_y(rail))  -- centered along Y
```

## center_z(s)

Centers a shape along Z around 0 (shifts by half its height).

```lua
local plate = extrude(rect(80, 40), 6)
emit(center_z(plate))  -- thickness goes from -3 to +3
```

## center_xy(s)

Centers a shape in the XY-plane (X and Y only).

```lua
local bracket = extrude(rect(120, 80), 8)
emit(center_xy(bracket)) -- XY centered, Z unchanged
```

## center_xyz(s)

Centers a shape in all axes (X, Y, and Z).

```lua
local housing = box(60, 40, 30)
emit(center_xyz(housing)) -- centered at the world origin
```

## center_to(s, cx, cy, cz)

Moves a shape so that its center lands exactly at (cx, cy, cz).

```lua
-- Place a pulley with its centroid at (100, 0, 25)
local pulley = union(
cylinder(20, 10),
center_z(cylinder(25, 4)))
emit(center_to(pulley, 100, 0, 25))
```

### Parameters

- s: Shape
- cx, cy, cz: number — target center coordinates
- **Returns** Shape — translated so its center is at (cx, cy, cz).

## Practical Example: Assembling Parts

```lua
--8<-- "docs/assets/lua/assembly.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/assembly.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## Tips & Gotchas

- **Centering is non-destructive**: each call returns a new, translated shape.
- **Center before transform**: use `center_xy` or `center_xyz` before `rotate_*` or `scale` for predictable results.
- **Precise placement**: combine `center_xy` / `center_xyz` with `translate`, or use `center_to` directly.
- **Assemblies**: center each sub-part first, then position them—this keeps your math simple and robust.
- **Order matters**: `rotate_z(translate(s, …))` ≠ `translate(rotate_z(s, …))`.
- **Right-hand rule**: positive rotation angles follow the right-hand grip convention.
- **Scaling**: only uniform scaling is supported at the moment.

Transformations are your main tools for arranging shapes into assemblies. Combine them with Booleans to turn simple primitives into complex mechanical parts.
