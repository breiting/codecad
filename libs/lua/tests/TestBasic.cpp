#include <gtest/gtest.h>

#include <ccad/lua/LuaEngine.hpp>

using namespace std;
using namespace ccad::lua;

TEST(TestLua, Basic) {
    LuaEngine e;
    std::string err;
    ASSERT_TRUE(e.Initialize(&err));
    e.RunString("emit(box(10,10,10))", &err);
    auto s = e.GetEmitted();
    ASSERT_TRUE((bool)s);
}
