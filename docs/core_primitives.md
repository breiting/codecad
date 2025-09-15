# Primitives

Primitives are the basic building blocks in CodeCAD. They represent simple 3D shapes that you can combine, transform, or modify to create more complex parts.

All primitives are parametric: you specify their size in code, and can later adapt them easily by changing parameters.

## box(width, depth, height)

Creates a rectangular box (a cuboid).

```lua
local p = box(20, 10, 5)
emit(p)
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
local c = cylinder(5, 20)
emit(c)
```

- diameter → cylinder diameter
- height → cylinder height

📷 Example:
A cylinder with radius 5 mm and height 20 mm

## sphere(diameter)

Creates a sphere centered in origin.

```lua
local s = sphere(10)
emit(s)
```

- diameter → sphere diameter

📷 Example:
A 10 mm radius sphere

## cone(d1, d2, height)

Creates a conical or frustum-shaped solid.

```lua
local co = cone(10, 5, 20)
emit(co)
```

- d1 → bottom diameter
- d2 → top diameter (set to 0 for a true cone)
- height → cone height

📷 Example:
A cone tapering from radius 10 mm to 5 mm over 20 mm height

## wedge(dx, dy, dz, ltx)

Creates a wedge — essentially a box with slanted top faces.

```lua
local w = wedge(20, 10, 10, 5)
emit(w)
```

- dx, dy, dz → base box dimensions
- ltx → shift of left corner

📷 Example:
A block 20 × 10 × 10 mm with its top sloped

## hex_prism(diameter, height)

Creates a regular hexagonal prism (like a bolt head).

```lua
local h = hex_prism(5, 10)
emit(h)
```

- diameter → distance from flat to flat side (across)
- height → prism height

📷 Example:
A hex prism with radius 5 mm and height 10 mm

## rod(diameter, height)

Creates a simple cylindrical rod (alias for a cylinder, but semantically useful for mechanical parts).

```lua
local r = rod(2.5, 40)
emit(r)
```

- diameter → rod diameter
- height → rod length

📷 Example:
A 2.5 mm radius, 40 mm long rod

## Tips for Working with Primitives

- Start simple: Every complex part begins as a box, cylinder, or combination.
- Think parametrically: Store dimensions in variables, so you can easily adapt them later.
- Combine with Booleans: Use difference, union, and intersection to cut holes, join parts, or trim shapes.
