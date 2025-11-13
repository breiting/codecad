# Primitives

Primitives are the basic building blocks in CodeCAD. They represent simple 3D shapes that you can combine, transform, or modify to create more complex parts.

All primitives are parametric: you specify their size in code, and can later adapt them easily by changing parameters.

!!! note "Model resolution"

    All the models are generated below with deflection 0.2 to lower the file size.

## box(width, depth, height)

Creates a rectangular box (a cuboid).

```lua
--8<-- "docs/assets/lua/cube.lua"
```

- width → size along the X axis
- depth → size along the Y axis
- height → size along the Z axis

<div class="stl-viewer"
     data-src="/assets/models/cube.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## cylinder(diameter, height)

Creates a vertical cylinder centered in x/y

```lua
--8<-- "docs/assets/lua/cylinder.lua"
```

- diameter → cylinder diameter
- height → cylinder height

<div class="stl-viewer"
     data-src="/assets/models/cylinder.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## sphere(diameter)

Creates a sphere centered in origin.

```lua
--8<-- "docs/assets/lua/sphere.lua"
```

- diameter → sphere diameter

<div class="stl-viewer"
     data-src="/assets/models/sphere.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## cone(d1, d2, height)

Creates a conical or frustum-shaped solid.

```lua
--8<-- "docs/assets/lua/cone.lua"
```

- d1 → bottom diameter
- d2 → top diameter (set to 0 for a true cone)
- height → cone height

<div class="stl-viewer"
     data-src="/assets/models/cone.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## wedge(dx, dy, dz, ltx)

Creates a wedge — essentially a box with slanted top faces.

```lua
--8<-- "docs/assets/lua/wedge.lua"
```

- dx, dy, dz → base box dimensions
- ltx → shift of left corner

<div class="stl-viewer"
     data-src="/assets/models/wedge.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## hex_prism(diameter, height)

Creates a regular hexagonal prism (like a bolt head).

```lua
--8<-- "docs/assets/lua/hex_prism.lua"
```

- diameter → distance from flat to flat side (across)
- height → prism height

<div class="stl-viewer"
     data-src="/assets/models/hex_prism.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## poisson_plate(spec, thickness)

Generates an organic “bubble” plate using Poisson disk sampling.  
The result is a perforated metal plate with circular holes that look like rising bubbles.

```lua
--8<-- "docs/assets/lua/poisson_plate.lua"
```

Key fields on `PoissonDiskSpec`:

- width → plate width in mm (X direction)
- height → plate height in mm (Y direction)
- thickness → plate thickness in mm
- margin → border margin where no bubbles may appear
- target_points → approximate number of bubbles to try to place
- r_min / r_max → minimum/maximum bubble radius
- min_gap → minimum distance between adjacent bubbles
- radius_falloff → how fast bubble size shrinks towards the top
- density_falloff → how fast bubble count thins out towards the top

<div class="stl-viewer"
     data-src="/assets/models/poisson_plate.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

## Tips for Working with Primitives

- Start simple: Every complex part begins as a box, cylinder, or combination.
- Think parametrically: Store dimensions in variables, so you can easily adapt them later.
- Combine with Booleans: Use difference, union, and intersection to cut holes, join parts, or trim shapes.
