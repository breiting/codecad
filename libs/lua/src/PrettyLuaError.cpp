#include "ccad/lua/PrettyLuaError.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return {};
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

std::string color(const char* code, const std::string& s) {
    return std::string("\x1b[") + code + "m" + s + "\x1b[0m";
}
std::string bold(const std::string& s) {
    return color("1", s);
}
std::string red(const std::string& s) {
    return color("31", s);
}
std::string yellow(const std::string& s) {
    return color("33", s);
}
std::string cyan(const std::string& s) {
    return color("36", s);
}
std::string dim(const std::string& s) {
    return color("2", s);
}

std::vector<std::string> splitLines(const std::string& t) {
    std::vector<std::string> out;
    std::istringstream is(t);
    std::string line;
    while (std::getline(is, line)) out.push_back(line);
    return out;
}

std::string makeContext(const std::string& file, int line, int pad = 2) {
    if (file.empty() || line <= 0) return {};
    auto txt = readFile(file);
    if (txt.empty()) return {};
    auto lines = splitLines(txt);
    int idx = line - 1;
    int a = std::max(0, idx - pad);
    int b = std::min((int)lines.size() - 1, idx + pad);
    std::ostringstream oss;
    for (int i = a; i <= b; i++) {
        bool mark = (i == idx);
        oss << (mark ? yellow("▶ ") : "  ");
        oss << cyan(std::to_string(i + 1)) << "  " << lines[i] << "\n";
        if (mark) {
            // optional: caret line
        }
    }
    return oss.str();
}

}  // namespace

namespace ccad::lua {

LuaErrorPretty FormatLuaError(lua_State* L, const sol::error& e) {
    (void)L;  // optional: luaL_traceback
    LuaErrorPretty pe;

    std::string msg = e.what();
    pe.headline = bold(red("Lua error: ")) + red(msg);

    // simple frame extraction
    std::regex frameRe(R"(([^:\n]+\.lua):(\d+)(?:: in function '([^']+)')?)");
    std::smatch m;
    std::string s = msg;
    while (std::regex_search(s, m, frameRe)) {
        LuaFrame f;
        f.file = m[1].str();
        f.line = std::stoi(m[2].str());
        f.func = m.size() >= 4 ? m[3].str() : "";
        f.isUser = (f.file.find("parts/") != std::string::npos) || (f.file.rfind(".lua") != std::string::npos);
        pe.frames.push_back(f);
        s = m.suffix().str();
    }

    if (!pe.frames.empty()) {
        const auto& top = pe.frames.front();
        pe.context = makeContext(top.file, top.line, 2);
        pe.hint = "Hint: Open " + top.file + ":" + std::to_string(top.line) + " in your editor.";
    }
    return pe;
}

void PrintLuaErrorPretty(const LuaErrorPretty& pe) {
    std::cerr << "\n" << pe.headline << "\n\n";
    if (!pe.context.empty()) {
        std::cerr << bold("\nContext:\n") << pe.context << "\n";
    }
    if (!pe.frames.empty()) {
        std::cerr << bold("Stack trace:\n");
        for (const auto& f : pe.frames) {
            std::string line = "  at " + f.file + ":" + std::to_string(f.line);
            if (!f.func.empty()) line += " in function '" + f.func + "'";
            std::cerr << (f.isUser ? line : dim(line)) << "\n";
        }
        std::cerr << "\n";
    }
    if (!pe.hint.empty()) {
        std::cerr << yellow(pe.hint) << "\n\n";
    }
}

}  // namespace ccad::lua
