# CodeCAD

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
  libocct-data-exchange-dev libocct-foundation-dev libocct-modeling-algorithms-dev \
  libocct-modeling-data-dev libocct-ocaf-dev libocct-visualization-dev \
  lua5.4 liblua5.4-dev
```

### macOS (Homebrew)

```bash
brew install cmake lua opencascade
```

## Build

```bash
git clone <this-project>
cd codecad
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Run example

```bash
./cad build ../examples/simple.lua
```

Outputs:

- `simple.stl` and/or
- `simple.step`

## TODO

- code refactoring (architecture)
- add `extrude(rect(...), h)`
- Read and import STEP files
