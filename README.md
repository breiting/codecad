# CodeCAD — Code‑First Parametric CAD

**CodeCAD** is a tiny but robust, scriptable CAD engine. You write **Lua**, it builds **solids** using OpenCascade (OCCT).
It targets 3D printing, CNC and general design automation with a puristic, math‑friendly API.

- ⚙️ **C++17 core** with OCCT for solid modeling
- 🧩 **Lua scripting** via sol2
- 🧪 **Deterministic**: files in, meshes out (STL/STEP)
- 🧠 **LLM‑friendly**: small API surface, consistent naming

## Why?

Most GUI CAD tools are great for interactive work but bad at **parameters, automation and versioning**.
Text‑based models are easy to diff, review and reuse. CodeCAD embraces that: **code → geometry**.

## Features

- Primitives: `box`, `cylinder`, `sphere`, `wedge`, `hex_prism`
- Booleans: `union`, `difference`
- Transforms: `translate`, `rotate*`, `scale`
- Features: `fillet`, `chamfer`
- Draft: `section_outline`
- Sketch → Solid: `poly_xy`, `poly_xz`, `extrude`, `revolve`
- Gears: `gear_involute(z, m, thickness, bore, pressure_deg?)`
- IO: `emit`, `save_stl`, `save_step`
- Live-processing for hot-reloading

See **docs/LUA_API.md** for the complete reference.

## Build

Requirements:

- CMake ≥ 3.16
- C++17 compiler
- OpenCascade (OCCT) installed and discoverable (`find_package(OpenCASCADE)`)
- Lua 5.4 (headers + lib)

```bash
git clone https://github.com/breiting/codecad && cd codecad
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Usage

Build a script and write outputs to `out/`:

```bash
./build/ccad build examples/simple.lua -o build/out
```

Override parameters:

```bash
./build/ccad build examples/gear_involute.lua -o build/out   -D z=32 -D m=2 -D th=10 -D bore=5
```

Live rebuild on file changes:

```bash
./build/ccad live examples/simple.lua -o build/out
```

## Examples

- `examples/gear_involute.lua` — parametric spur gear (true involute)
- `examples/hose_adapter.lua` — revolve a profile to create a hose adapter
- `examples/m6_set.lua` — bolt + nut + washer
- `examples/terasse.lua` — wooden deck layout

Each file contains comments describing the parameters.

## Editor Support

- Lua LSP stubs in `types/cad.d.lua` for diagnostics and completion.
- Neovim users: make sure `Lua.workspace.library` includes `./types` and `./lib`.

## Contributing

- Read **docs/DEV.md** first.
- Keep Lua API changes reflected in **docs/LUA_API.md** and `types/cad.d.lua`.
- Prefer small, focused PRs with examples.

## License

MIT
