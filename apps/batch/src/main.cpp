#include <ccad/base/Shape.hpp>
#include <ccad/io/Export.hpp>
#include <ccad/lua/LuaEngine.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Options {
    fs::path inDir;
    fs::path outDir;
    double deflection = 0.2;  // Tesselation
    bool failFast = false;    // quit with first error
    bool quiet = false;       // less verbose
};

static void printUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " --in <dir> --out <dir> [--defl 0.2] [--fail-fast] [--quiet]\n";
}

std::shared_ptr<ccad::lua::LuaEngine> GetEngine() {
    auto engine = std::make_shared<ccad::lua::LuaEngine>();
    std::vector<std::string> paths = {"./lib/?.lua", "./lib/?/init.lua", "./vendor/?.lua", "./vendor/?/init.lua"};

    // Environment-Variable LUA_PATH (optional)
    if (const char* env = std::getenv("LUA_PATH")) {
        std::stringstream ss(env);
        std::string token;
        while (std::getline(ss, token, ';')) {
            if (!token.empty()) paths.push_back(token);
        }
    }

    engine->SetLibraryPaths(paths);

    std::string err;
    if (!engine->Initialize(&err)) {
        throw std::runtime_error(std::string("CoreEngine init failed: ") + err);
    }
    return engine;
}

static std::optional<Options> parseArgs(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--in" && i + 1 < argc) {
            opt.inDir = argv[++i];
        } else if (a == "--out" && i + 1 < argc) {
            opt.outDir = argv[++i];
        } else if (a == "--defl" && i + 1 < argc) {
            opt.deflection = std::stod(argv[++i]);
        } else if (a == "--fail-fast") {
            opt.failFast = true;
        } else if (a == "--quiet") {
            opt.quiet = true;
        } else if (a == "--help" || a == "-h") {
            printUsage(argv[0]);
            return std::nullopt;
        } else {
            std::cerr << "Unknown arg: " << a << "\n";
            printUsage(argv[0]);
            return std::nullopt;
        }
    }
    if (opt.inDir.empty() || opt.outDir.empty()) {
        printUsage(argv[0]);
        return std::nullopt;
    }
    return opt;
}

int main(int argc, char** argv) {
    auto parsed = parseArgs(argc, argv);
    if (!parsed) return 2;
    Options opt = *parsed;

    if (!fs::exists(opt.inDir) || !fs::is_directory(opt.inDir)) {
        std::cerr << "ERROR: input dir does not exist or is not a directory: " << opt.inDir << "\n";
        return 3;
    }
    fs::create_directories(opt.outDir);

    std::vector<fs::path> luaFiles;
    for (auto& p : fs::recursive_directory_iterator(opt.inDir)) {
        if (!p.is_regular_file()) continue;
        if (p.path().extension() == ".lua") {
            luaFiles.push_back(p.path());
        }
    }
    if (!opt.quiet) {
        std::cout << "Found " << luaFiles.size() << " Lua files in " << opt.inDir << "\n";
    }

    size_t ok = 0, fail = 0;
    std::chrono::milliseconds totalMs{0};

    for (const auto& lua : luaFiles) {
        fs::path rel = fs::relative(lua, opt.inDir);
        fs::path out = opt.outDir / rel;
        out.replace_extension(".stl");
        fs::create_directories(out.parent_path());

        if (!opt.quiet) {
            std::cout << "[RUN] " << rel.string() << " -> " << out.string() << "\n";
        }

        auto engine = GetEngine();

        auto t0 = std::chrono::steady_clock::now();
        bool ran = engine->RunFile(lua.string());
        auto t1 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
        totalMs += ms;

        if (!ran) {
            std::cerr << "  ERROR in " << rel.string() << std::endl;
            fail++;
            if (opt.failFast)
                break;
            else
                continue;
        }

        auto emitted = engine->GetEmitted();
        if (!emitted) {
            std::cerr << "  ERROR: script did not call emit(...)\n";
            fail++;
            if (opt.failFast)
                break;
            else
                continue;
        }

        auto params = ccad::lua::GetTriangulationParameters();
        params.linearDeflection = opt.deflection;
        if (!ccad::io::SaveSTL(emitted.value(), out.string(), params)) {
            std::cerr << "  ERROR: failed to write STL\n";
            fail++;
            if (opt.failFast)
                break;
            else
                continue;
        }

        if (!opt.quiet) {
            std::cout << "  OK (" << ms.count() << " ms)\n";
        }
        ok++;
    }

    std::cout << "\nSummary: OK=" << ok << "  FAIL=" << fail << "  Total=" << totalMs.count()
              << " ms  Avg=" << (ok + fail > 0 ? totalMs.count() / (ok + fail) : 0) << " ms/file\n";

    return (fail == 0) ? 0 : 1;
}
