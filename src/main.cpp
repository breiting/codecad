#include <filesystem>
#include <iostream>
#include <sol/sol.hpp>
#include <string>

#include "IO/Export.hpp"
#include "Runtime/LuaBindings.hpp"

namespace fs = std::filesystem;

struct Cmd {
    std::string subcommand;  // "build"
    std::string script;
    fs::path outdir = "out";
};

static void PrintUsage() {
    std::cout << "Usage: cad build <script.lua> [-o <outdir>]\n";
}

static bool ParseArgs(int argc, char** argv, Cmd& cmd) {
    if (argc < 3) return false;
    cmd.subcommand = argv[1];
    cmd.script = argv[2];
    for (int i = 3; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-o" || a == "--out") && i + 1 < argc) {
            cmd.outdir = argv[++i];
        } else {
            std::cerr << "Unknown arg: " << a << "\n";
            return false;
        }
    }
    return cmd.subcommand == "build";
}

int main(int argc, char** argv) {
    Cmd cmd;
    if (!ParseArgs(argc, argv, cmd)) {
        PrintUsage();
        return 1;
    }

    try {
        fs::create_directories(cmd.outdir);
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::package);

        Runtime::LuaBindings bindings;
        bindings.Register(lua);

        // Provide a small helper in Lua to set output directory
        lua["__OUTDIR"] = cmd.outdir.string();

        // Run user's script
        sol::load_result chunk = lua.load_file(cmd.script);
        if (!chunk.valid()) {
            sol::error err = chunk;
            std::cerr << "Lua load error: " << err.what() << "\n";
            return 1;
        }
        sol::protected_function_result result = chunk();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua runtime error: " << err.what() << "\n";
            return 1;
        }

        // If user emitted a shape but didn't save it, write a default STL
        auto emitted = bindings.GetEmitted();
        if (emitted) {
            const auto stem = fs::path(cmd.script).stem().string();
            const fs::path out_stl = cmd.outdir / (stem + ".stl");
            IO::SaveSTL(emitted, out_stl.string(), 0.1);
            std::cout << "Build succeeded.\n";
        } else {
            std::cout << "Script didn't call emit(...). Nothing to build.\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}
