# codecad

A simple and puristic approach for a Lua-based parametric CAD modeling software.

Tiny MVP for a code-first CAD tool:

- C++17 core
- Lua scripting via sol2
- OpenCascade for geometry
- Commands: `box`, `cylinder`, `union`, `emit`, `save_stl`, `save_step`

## Requirements

Install dependencies:

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y build-essential cmake git \
  liboce-foundation-dev liboce-modeling-dev liboce-ocaf-dev liboce-visualization-dev \
  libocct-data-exchange-dev libocct-foundation-dev libocct-modeling-algorithms-dev \
  libocct-modeling-data-dev libocct-ocaf-dev libocct-visualization-dev \
  lua5.4 liblua5.4-dev
```

> Note: On newer distros OpenCascade packages are named `libocct-...`. On older ones, `liboce-...` (OCE). Prefer OCCT.

### macOS (Homebrew)

```bash
brew install cmake lua opencascade
```

## Build

```bash
git clone <this-project>
cd codecad
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run example

```bash
cmake --build build --target run_example
# or directly:
./build/cad build examples/simple.lua
```

Outputs:

- `simple.stl`
- `simple.step`

## Notes

This is a minimal first example. Next steps:

- add `translate/rotate`
- add `difference`
- add `extrude(rect(...), h)` etc.
- implement a file watch / live preview loop
