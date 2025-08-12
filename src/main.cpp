#include <filesystem>
#include <iostream>
#include <runtime/SolConfig.hpp>
#include <string>
#include <thread>

#include "io/Export.hpp"
#include "runtime/LuaBindings.hpp"

namespace fs = std::filesystem;

struct Cmd {
    std::string subcommand;  // "build", "live"
    std::string script;
    fs::path outdir = "out";
};

static void PrintUsage() {
    std::cout << "Usage: cad <command> <script.lua> [-o <outdir>]\n";
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
    return cmd.subcommand == "build" || cmd.subcommand == "live";
}

static bool BuildOnce(const Cmd& cmd, runtime::LuaBindings& bindings) {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::package);

    // add additional paths
    std::string libPath = "./lib/?.lua;./lib/?/init.lua;./vendor/?.lua;./vendor/?/init.lua;";
    std::string current = lua["package"]["path"];
    lua["package"]["path"] = libPath + current;

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
        io::SaveSTL(emitted, out_stl.string(), 0.1);
        std::cout << "Build succeeded.\n";
    }
    return true;
}

int main(int argc, char** argv) {
    Cmd cmd;
    if (!ParseArgs(argc, argv, cmd)) {
        PrintUsage();
        return 1;
    }

    try {
        fs::create_directories(cmd.outdir);

        if (cmd.subcommand == "build") {
            runtime::LuaBindings bindings;
            BuildOnce(cmd, bindings);
            return 0;
        }

        // -- live mode
        runtime::LuaBindings bindings;
        auto last = fs::last_write_time(cmd.script);
        std::cout << "Watching " << cmd.script << " (Ctrl-C to stop)\n";
        BuildOnce(cmd, bindings);  // initial

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            try {
                auto now = fs::last_write_time(cmd.script);
                if (now != last) {
                    last = now;
                    std::cout << "\nChange detected. Rebuilding...\n";
                    bindings = runtime::LuaBindings{};  // reset emitted
                    BuildOnce(cmd, bindings);
                }
            } catch (...) {
                // ignore transient errors (file being written)
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}
