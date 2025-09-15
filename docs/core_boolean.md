# Boolean Operations

Boolean operations are the foundation of constructive solid geometry (CSG). They allow you to combine or subtract shapes to build complex parts from simple building blocks. In CodeCAD, Booleans work on any solid Shape.

## union(a, b, ...)

Merges two or more solids into a single shape. This is equivalent to a logical OR.

```lua
-- Two overlapping boxes fused together
local a = box(20, 20, 20)
local b = box(20, 20, 20)
b = translate(b, 10, 0, 0)
emit(union(a, b))
```

### Parameters

- a: Shape
- b: Shape
- ...: additional Shapes (optional)
- **Returns** Shape — the combined solid.

Use case: Build a larger part from smaller modules (e.g., base plate + reinforcement ribs). You can also union not intersecting parts in order to build groups.

## difference(a, b)

Subtracts shape b from a. The result is A minus B.

```lua
-- A box with a cylindrical hole
local base = box(40, 40, 20)
local hole = cylinder(8, 20)
hole = translate(hole, 20, 20, 0)
emit(difference(base, hole))
```

### Parameters

- a: Shape — the main solid (minuend).
- b: Shape — the cutting solid (subtrahend).
- **Returns** Shape — the remaining solid after subtraction.

Use case: Holes, cutouts, slots, and pockets.

## intersection(a, b)

Keeps only the overlapping volume of a and b. This is equivalent to a logical AND.

```lua
-- Lens-like shape: overlap of two spheres
local s1 = sphere(15)
local s2 = sphere(15)
s2 = translate(s2, 10, 0, 0)
emit(intersection(s1, s2))
```

### Parameters

- a: Shape
- b: Shape
- **Returns** Shape — the common solid volume.

Use case: Useful for trimming shapes to a boundary or creating fillets by overlap.

## Practical Example: Mounting Bracket

```lua
--8<-- "docs/assets/lua/boolean.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/boolean.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## Tips & Gotchas

- Order matters in difference: difference(a, b) ≠ difference(b, a).
- Union can take many shapes: union(a, b, c, d) is valid.
- Clean geometry: For best results, ensure inputs overlap or touch meaningfully. Thin intersections may lead to unstable results.
- Combine with transforms: Position parts with translate/rotate\_\* before applying Booleans.

Boolean operations let you think like a sculptor: add with union, carve with difference, and trim with intersection. Together, they form the backbone of parametric CAD in CodeCAD.
