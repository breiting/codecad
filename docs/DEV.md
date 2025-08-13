# Developer Guide

This document explains the code layout, build, and contribution guidelines for **CodeCAD**.

## Project Layout

```
inc/                 # Public headers (C++)
  geo/               # Geometry primitives and operations (OCCT wrappers)
  io/                # Import/export helpers (STL/STEP)
  runtime/           # Lua bindings, Sol2 config, runtime utilities
src/                 # Implementations
  lua/               # Modular Lua binding registrations (Bind*.cpp)
examples/            # Lua examples
lib/                 # Reusable Lua modules
types/               # Lua language server stubs (sumneko / LuaLS)
docs/                # Documentation
```

## CMake Targets

- **ccad** — CLI that loads Lua, executes a script, processes and/or writes STL/STEP.

## Coding Style

- C++17, Google style via `.clang-format`.
- Classes in `CamelCase`, methods `Capitalised()`, private members `m_Var`.
- Document all public headers with **Doxygen** (`///`).
- Prefer **clear names** over abbreviations; e.g. `radiusOuter` not `rOut`.

## Error Handling

- OCCT routines may throw `Standard_Failure`. Catch at API boundaries and
  raise a `std::runtime_error` for Lua with a descriptive message.
- Boolean operations (fuse, cut) may return invalid shapes; check `IsNull()` and
  fall back gracefully when sensible.

## Lua Bindings

Bindings are split into small modules in `src/lua`:

- `BindPrimitives.cpp` — `box`, `cylinder`, `hex_prism`
- `BindTransforms.cpp` — `translate`, `rotate_*`, `scale`
- `BindBooleans.cpp` — `union`, `difference`
- `BindFeatures.cpp` — `fillet`, `chamfer`
- `BindSketch.cpp` — `poly_xy`, `poly_xz`
- `BindConstruct.cpp` — `extrude`, `revolve`
- `BindGears.cpp` — `gear_involute`
- `BindIO.cpp` — `emit`, `save_stl`, `save_step`, `emit_assembly`

`LuaBindings::Register()` only orchestrates these.

## Contributing

- Add tests for new functionality (if non-trivial).
- Keep examples short and descriptive.
- Update `types/cad.d.lua` when changing Lua API.
