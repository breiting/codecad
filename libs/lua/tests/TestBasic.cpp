#include <gtest/gtest.h>

#include <ccad/lua/LuaEngine.hpp>

using namespace std;
using namespace ccad::lua;

TEST(TestLua, Basic) {
    LuaEngine e;
    ASSERT_TRUE(e.Initialize());
    e.RunString("emit(box(10,10,10))");
    auto s = e.GetEmitted();
    ASSERT_TRUE((bool)s);
}
