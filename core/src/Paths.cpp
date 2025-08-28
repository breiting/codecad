#include "io/Paths.hpp"

#include <fstream>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#endif

namespace io {

std::filesystem::path ExecutablePath() {
    char buf[4096] = {0};
#ifdef __APPLE__
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        return std::filesystem::canonical(std::filesystem::path(buf));
    }
    return {};
#elif __linux__
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        return std::filesystem::canonical(std::filesystem::path(buf));
    }
    return {};
#else
    return {};
#endif
}

std::vector<std::string> DefaultInstallLuaPathsFromExe() {
    namespace fs = std::filesystem;
    std::vector<std::string> out;
    fs::path exe = ExecutablePath();
    if (exe.empty()) return out;
    fs::path prefix = exe.parent_path().parent_path();  // .../bin -> prefix
    fs::path share = prefix / "share" / "codecad";
    auto add = [&](const fs::path& base) {
        out.push_back((base / "?.lua").string());
        out.push_back((base / "?/init.lua").string());
    };
    add(share / "lib");
    add(share / "types");
    return out;
}

std::string DefaultInstallFontsPath() {
    namespace fs = std::filesystem;
    fs::path exe = ExecutablePath();
    if (exe.empty()) return fs::current_path();
    fs::path prefix = exe.parent_path().parent_path();  // .../bin -> prefix
    return prefix / "share" / "codecad" / "fonts";
}

bool WriteTextFile(const std::filesystem::path& p, const std::string& content, std::string* err) {
    try {
        std::filesystem::create_directories(p.parent_path());
        std::ofstream f(p, std::ios::binary);
        if (!f) {
            if (err) *err = "cannot open file for write";
            return false;
        }
        f << content;
        return true;
    } catch (const std::exception& e) {
        if (err) *err = e.what();
        return false;
    }
}
}  // namespace io
