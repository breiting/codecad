#pragma once
#include <sol/sol.hpp>
#include <string>
#include <vector>

namespace ccad::lua {

struct LuaFrame {
    std::string file;
    int line = 0;
    std::string func;
    bool isUser = true;  // internal or user library
};

struct LuaErrorPretty {
    std::string headline;          // "Lua Error: <message>"
    std::vector<LuaFrame> frames;  // call stack (top -> bottom)
    std::string context;           // code excerpt with caret or highlight
    std::string hint;              // optional tip
};

LuaErrorPretty FormatLuaError(lua_State* L, const sol::error& e);
void PrintLuaErrorPretty(const LuaErrorPretty& pe);

}  // namespace ccad::lua
