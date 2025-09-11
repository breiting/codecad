# CodeCAD — Parametric CAD in Code

**CodeCAD** is a lightweight, code-first parametric CAD tool.
You write **Lua scripts**, CodeCAD builds **solids** using the robust [OpenCascade](https://www.opencascade.com/) kernel (OCCT) — with live preview, reusable parts, and a workflow that feels as natural as editing code in your favorite editor.

Please find more information on our documentation site [https://www.codecad.xyz](https://www.codecad.xyz).

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
- Parametrically adapt designs, do not start all over, if you want to change a small parameter

👉 If you love code, the command line, and reproducible workflows — CodeCAD is for you.

## 🏛 Software Architecture

### KERNEL

Pure C++ layer on top of OCCT: custom types (Shapes, TriMesh, Ops), Extrude/Revolve, Booleans, Fillet/Chamfer, Section, Triangulation, STL/STEP export. OCCT stays encapsulated internally. The kernel API does not expose any OCCT types.

### LUA

Binds the kernel API to Lua (via sol2). Provides a stable, curated scripting API (Primitives, Transforms, Ops, higher-level parts like Threads, Rods).

### PURE

Lightweight render engine (GLFW + GLM + ImGui). Accepts TriMeshes from Kernel/Lua/Service, renders them, displays status bar & dock panel.

### CLI

Project tool (`ccad init/parts/live/build/bom`, etc.). Can execute Lua/kernels locally; shows live geometry through Pure.

### LAB

Playground for new geometries/algorithms. Uses kernel and PURE for visualization.

> [!IMPORTANT]
> We are still in an early development stage. Please consider that the APIs are not yet stable and may change in future.

## 🚀 Getting Started

### Requirements

- CMake
- C++17 compiler
- OpenCascade (OCCT) installed (version 7.9.1)

### Build

```bash
git clone https://github.com/breiting/codecad
cd codecad
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
make install
```

### Scaffold a new project

```bash
# initialize new project
mkdir myproject && cd myproject
ccad init

# add a first part (the name is optional)

ccad parts add "Box"

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

## 📖 Editor Support

- Lua LSP stubs in types/ for completion & diagnostics
- For Neovim: include ./types and ./lib in your Lua.workspace.library

## 🤝 Contributing

You are welcome to contribute to this project. I am developing under MacOS and Linux. However, I cannot test with Windows. So volunteers are welcome to support all three platforms.
Any feature requests should be filed directly in Github.

## Links

- [OpenCASCADE](https://github.com/Open-Cascade-SAS/OCCT)

## 📜 License

[MIT License](LICENSE)
