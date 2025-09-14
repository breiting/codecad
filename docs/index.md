# CodeCAD - Parametric CAD with Lua

CodeCAD is a modern, code-driven CAD system designed for makers, engineers, and researchers who want **simplicity, power, and reproducibility** in 3D modeling.

Instead of clicking through menus, you describe your parts in **clean Lua code**. Models live in plain text, can be versioned with Git, and preview instantly in the built-in viewer.

Think of CodeCAD as a catalyst: a tool that transforms your ideas into real-world objects — whether for 3D printing, woodworking, metalworking, or larger construction projects. Its workflow is designed to be as frictionless and direct as possible.

## Why CodeCAD?

- ✨ **Simplicity first** – Minimal API, easy to learn. A box in just one line of code.
- ⚡ **Frictionless workflow** – Edit [Lua](https://lua.org) → see your model update live.
- 🛠️ **Industrial kernel** – Built on [OpenCascade](https://www.opencascade.com/) (OCCT), a proven geometry engine for serious CAD.
- 💾 **Reproducible** – Everything is text-based and versionable. Git is your history of design.
- 🎯 **Developer-friendly** – If you can program, you can model. Treat CAD like software engineering.
- 📝 **Editor freedom** – Works great in any text editor; optimized for [neovim](https://neovim.io).

## Why Lua?

- 🐚 Small, clean, embeddable scripting language.
- 📦 No dependencies, works everywhere.
- 🧩 Concise enough to act like a DSL, but still a real programming language.

With Lua, you describe parts with code that is **easy to read, easy to share, and easy to maintain**.

!!! info "🚀 **Code your designs. Print your ideas.**"

## Who is CodeCAD for?

- 🧑‍🔧 **Makers** – Design printable parts and assemblies faster.
- 🏭 **Engineers** – Prototype mechanisms, gears, bolts, and housings parametrically.
- 🔬 **Researchers** – Explore geometry and manufacturing processes in a programmable, reproducible way.
- 🔬 **Software developers** – Apply software engineering mindset to CAD modeling.

## First example: A simple hex knob

```lua
local hex_across_flats = 13 -- mm
local hex_height = 6        -- mm

local knob_diameter = 20    -- mm
local knob_height = 10      -- mm

local hex = hex_prism(hex_across_flats, hex_height)
hex = translate(hex, 0, 0, knob_height - hex_height)

local cyl = cylinder(knob_diameter, knob_height)

emit(difference(cyl, hex))
```

<div class="stl-viewer"
     data-src="/assets/models/hexnut_nobb.stl"
     data-color="#4b9fea"
     data-grid="true"
     data-controls="true"
     data-autorotate="true">
</div>

💡 With CodeCAD, you’re not just designing parts – you’re **writing the blueprint of your ideas**.
