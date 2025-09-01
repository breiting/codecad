# Developer Guide

This document explains the code layout, build, and contribution guidelines for **CodeCAD**.

## CMake Targets

- **ccad** — CLI that loads Lua, executes a script, processes and/or writes STL/STEP and starts live viewer

## Coding Style

- C++17, Google style via `.clang-format`.
- Classes in `CamelCase`, methods `Capitalised()`, private members `m_Var`.
- Document all public headers with **Doxygen** (`/**`).
- Prefer **clear names** over abbreviations; e.g. `radiusOuter` not `rOut`.

## Error Handling

- OCCT routines may throw `Standard_Failure`. Catch at API boundaries and
  raise a `std::runtime_error` for Lua with a descriptive message.
- Boolean operations (fuse, cut) may return invalid shapes; check `IsNull()` and
  fall back gracefully when sensible.

## Contributing

- Add tests for new functionality (if non-trivial).
- Keep examples short and descriptive.
- Update `types/?.d.lua` when changing Lua API.
