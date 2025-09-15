# Features

Features let you add finishing touches to your parts by softening or beveling sharp edges. CodeCAD provides both global (all edges) and selective (filtered edges) variants for fillets and chamfers.

## Global Edge Modifiers

### fillet_all(s, r)

Applies a rounded fillet of constant radius r to all eligible edges of a shape.

```lua
--8<-- "docs/assets/lua/fillet_all.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/fillet_all.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

!!! tip

    Use `fillet_all` for quick prototypes. For precise control, switch to selective edge queries (see below).

### chamfer_all(s, d)

Applies a straight chamfer of distance d to all eligible edges of a shape.

```lua
--8<-- "docs/assets/lua/chamfer_all.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/chamfer_all.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## Selective Edge Control

For more refined designs, you can query edges based on location, geometry, or orientation, then apply fillet or chamfer only where needed.

### Edge Query Workflow

1. Start with edges(shape) to create an EdgeQuery.
2. Apply filters such as `on_box_side`, `geom`, `parallel`, `dihedral_between`, `length_between`.
3. Collect results with `.collect()`.
4. Pass the resulting EdgeSet into fillet or chamfer.

#### Example: Fillet only vertical edges

```lua
--8<-- "docs/assets/lua/edge1.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/edge1.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

#### Example: Chamfer only top edges

```lua
--8<-- "docs/assets/lua/edge2.lua"
```

<div class="stl-viewer"
     data-src="/assets/models/edge2.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

### Edge Filters

- `on_box_side(side)`
  Selects edges on one side of the shape’s bounding box.
  Valid values: `xmin`, `xmax`, `ymin`, `ymax`, `zmin`, `zmax`.
- `geom(kind)`
  Select edges by geometric type: `line` (straight) or `circle` (arcs).
- `parallel(axis, tol_deg?)`
  Select edges whose tangent is parallel to `x`, `y`, or `z` within tolerance (default 3°).
- `dihedral_between(min_deg, max_deg)`
  Select edges based on the angle between adjacent faces.
- Example: sharp edges (< 100°), or shallow bends (> 170°).
- `length_between(min_mm, max_mm)`
  Select edges by their length interval.

## Tips & Gotchas

- Global vs selective: `fillet_all`/`chamfer_all` are fast, but selective queries give you design intent.
- Edge eligibility: Some complex intersections may not support filleting/chamfering.
- Combine filters: Queries are chainable (e.g., :on_box_side("zmax"):parallel("x")).
- Dihedral filter: Great for distinguishing sharp corners from nearly flat edges.
- Iteration-friendly: Queries return a new builder each time—safe for reuse.

With global and selective tools, you can decide whether to quickly soften all edges or carefully refine just the functional ones. This balance gives CodeCAD the power of parametric design with real-world manufacturing detail.
