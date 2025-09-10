# CodeCAD – Parametric CAD with Lua

**CodeCAD** is a lightweight, scriptable CAD tool for makers, engineers, and researchers.
You model with **Lua** (or C++ in the kernel), view the result live in the **PURE** viewer,
and export **STL/STEP** for 3D printing or further CAD workflows.

## Why CodeCAD?

- 💡 **Parametric**: Every dimension is a variable – change numbers, not geometry.
- 🧠 **Lua-first**: A clear, simple language instead of a proprietary DSL.
- 🧩 **Extensible**: C++ kernel (OCCT), Lua bindings, modular PURE viewer (GLFW/GL).
- 🧵 **Practical features**: Printable threads, pipe adapters, Bezier/BSpline, fillet/chamfer with edge selector.
- 🛠️ **Open Source**: Build your own tooling – or use ready-made building blocks.

## What can I build?

- Mechanical parts (bolts, nuts, jars with screw caps, brackets, gears)
- Organic shapes (vases via Bezier/BSpline + revolve)
- Adapters (pipe transitions), bent plates, etc.

## Quick Impression (Lua)

```lua
-- Simple parametric box with rounded edges
local w, d, h = 100, 60, 20
local b = box(w, d, h)
b = fillet(b, 1)
emit(b)
```
