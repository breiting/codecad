# Installation

CodeCAD is developed and tested primarily on **macOS** and **Linux**.  
Windows support is not yet available — if you want to help porting CodeCAD to Windows, contributions are welcome!

## Requirements

Before building CodeCAD, make sure you have the following installed:

- A modern **C++ compiler** with C++17 support (e.g. `clang++` ≥ 12, `g++` ≥ 9)
- **CMake** ≥ 3.28
- **OpenCASCADE** 7.9.1 (provides the geometric modeling kernel)
- Standard development libraries

## Installing OpenCASCADE

OpenCASCADE forms the core of CodeCAD’s kernel. You need to install it together with some development dependencies.

### Linux (Ubuntu/Debian)

Install via APT:

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    liblua5.4-dev \
    xorg-dev libgl1-mesa-dev libglu1-mesa-dev \
    libtbb-dev libtbb12 \
    libocct-data-exchange-dev \
    libocct-foundation-dev \
    libocct-modeling-algorithms-dev \
    libocct-modeling-data-dev
```

Tip: On other Linux distributions (Fedora, Arch, ...) the package names may differ.
Refer to your distribution’s package manager and look for opencascade or occt packages.

## macOS (Homebrew)

Install via Homebrew:

```bash
brew update
brew install cmake opencascade
```

Note: CodeCAD is regularly tested on macOS with OCCT 7.9.1 via Homebrew.
If you have an older OCCT version, compilation may fail.

## Build

Clone the repository and build:

```bash
git clone https://github.com/breiting/codecad
cd codecad
mkdir build && cd build
cmake ..
make -j$(nproc)   # On macOS: use `make -j$(sysctl -n hw.ncpu)`
sudo make install
```

!!! note “First CMake run”

    The first run of `cmake ..` may take longer because external dependencies are downloaded and configured.
    Subsequent runs will be faster.

## Post-installation

After `make install`, ensure the target `bin` directory is available in your `$PATH`.
You can check with:

```
ccad --help
```

If the installation was successful, you should see the CodeCAD CLI help. 🎉
You are now ready to jump into [Getting Started](getting_started.md).
