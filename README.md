# CodeCAD — Parametric CAD in Code

**CodeCAD** is a lightweight, code-first parametric CAD tool.
You write **Lua scripts**, CodeCAD builds **solids** using the robust [OpenCascade](https://www.opencascade.com/) kernel — with live preview, reusable parts, and a workflow that feels as natural as editing code in your favorite editor.

[![CodeCAD Demo](https://img.youtube.com/vi/xyIz4Y3gc14/hqdefault.jpg)](https://www.youtube.com/watch?v=xyIz4Y3gc14)

- ⚡ **Live**: code in your editor, geometry updates instantly in an integrated viewer
- 🧩 **Composable**: split designs into parts and reuse them
- 📐 **Parametric**: adjust dimensions by changing a single parameter
- 🛠 **For makers**: designed with 3D printing and woodworking in mind
- 🌱 **Open**: built on C++17, Lua, and OpenCascade

## ✨ Why CodeCAD?

Most GUI-based CAD tools are powerful, but:

- 🌀 Steep learning curves
- 💸 Expensive licenses
- 🔒 Hard to automate, version, and share

With CodeCAD you:

- Model in **pure code** (with Lua)
- Get **live preview** in the integrated viewer
- Keep your models **small, diff-able, and version-controlled** in Git
- Parametrically adapt designs: change `M6` → `M8` and your screw updates

👉 If you love code, the command line, and reproducible workflows — CodeCAD is for you.

## 🏛 Architecture

```text
┌────────────┐      ┌────────────┐      ┌───────────────┐
│   core     │─────▶│    pure    │─────▶│     main      │
│ LuaEngine  │      │ Viewer     │      │ CLI frontend  │
│ OCCT, STL  │      │ GLFW+ImGui │      │ ccad commands │
└────────────┘      └────────────┘      └───────────────┘

	•	core — C++ engine with Lua bindings, geometry, triangulation & STL export
	•	pure — A simple GLFW-based render engine optimized for CodeCAD with ImGui controls
	•	main — CLI: ccad init, ccad add, ccad live, ccad build
```

## 🚀 Getting Started

### Requirements

- CMake ≥ 3.16
- C++17 compiler
- OpenCascade (OCCT) installed
- Lua 5.4 (headers + libs)

### Build

```bash
git clone https://github.com/breiting/codecad
cd codecad
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cd build && make install
```

### Scaffold a new project

```bash
# initialize new project
mkdir myproject && cd myproject
ccad init

# add a first part (the name is optional)

ccad add --name "box"

# run live viewer
ccad live
```

This opens the viewer: edit parts/box.lua in your editor and see geometry update live.

## 📦 Project Structure

```text
myproject/
├── project.json # project metadata (name, units, params, materials, parts…)
├── parts/ # Lua scripts for parts
│ └── box.lua # each part is self-contained
└── generated/ # generated meshes (STL, STEP…)
```

Parts are written in Lua, e.g.:

```lua
-- parts/box.lua
local c = box(10, 20, 5)
emit(c)
```

## 🖼 Example

Parametric nut block:

```lua
local size = param("size", 20)
local hole = param("hole", 6)

local b = box(size, size, size/2)
local cut = cylinder(hole, size)
emit(difference(b, cut))
```

Change hole from 6 → 8 and regenerate — instantly updated.

## 🌱 Roadmap

- 📚 Tutorials & docs
- 🪵 Library extensions: codecad-wood, codecad-struct for domain-specific libraries
- 🔩 Threading functions (inner & outer threads)
- 🌍 Community support via Patreon

## 📖 Editor Support

- Lua LSP stubs in types/ for completion & diagnostics
- For Neovim: include ./types and ./lib in your Lua.workspace.library

## 🤝 Contributing

You are welcome to contribute to this project. I am developing under MacOS and Linux. However, I cannot test with Windows. So volunteers are welcome to support all three platforms.

Any feature requests should be filed directly in Github.

## 📜 License

[MIT License](LICENSE)
