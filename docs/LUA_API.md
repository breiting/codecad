# Lua API Reference

This file documents the public Lua functions exposed by **CodeCAD**.

> Units: millimeters unless stated otherwise.
> Angles: degrees.

## Parameters

### `param(name, default) -> any`

Read a parameter that can be overridden on the CLI using `-D name=value`.
Numbers and booleans are converted from strings automatically.

## Primitives

- `box(x, y, z) -> Shape` — axis-aligned box with size on X/Y/Z.
- `cylinder(d, h) -> Shape` — cylinder of diameter `d` and height `h`.
- `hex_prism(across_flats, h) -> Shape` — regular hexagonal prism.

## Booleans

- `union(a, b, ...) -> Shape`
- `difference(a, b) -> Shape`

## Transforms

- `translate(s, dx, dy, dz) -> Shape`
- `rotate_x(s, deg) -> Shape`
- `rotate_y(s, deg) -> Shape`
- `rotate_z(s, deg) -> Shape`
- `scale(s, factor) -> Shape`

## Features

- `fillet(s, r) -> Shape` — constant-radius fillet on all edges.
- `chamfer(s, d) -> Shape` — constant-distance chamfer on all edges.

> Large radii/distances can fail when the local geometry is too thin. Start small.

## Sketching

- `poly_xy(points, closed?) -> Shape(face)`  
  `points` is `{ {x,y}, ... }` or `{ {x=..., y=...}, ... }`.  
  If `closed` is omitted it defaults to `true`.

- `poly_xz(points, closed?, close_to_axis?) -> Shape(face)`  
  `points` is `{ {r,z}, ... }` in XZ. When `close_to_axis=true` the profile
  is connected to the Z axis to form a face suitable for `revolve(..., 360)`.

## Constructive

- `extrude(face, height) -> Shape(solid)` — extrude along +Z.
- `revolve(profile, angle_deg) -> Shape(solid)` — revolve around Z axis.

## Gears

- `gear_involute(z, module, thickness, bore, pressure_deg?) -> Shape(solid)`  
  Generates a robust involute spur gear. `pressure_deg` defaults to 20°.

## Pipeline

- `emit(shape)` — mark the final shape to be exported.
- `save_stl(shape, path)` — export STL.
- `save_step(shape, path)` — export STEP.
