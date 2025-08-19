#pragma once

#include <memory>

#include "LuaEngine.hpp"

class App {
   public:
    App();
    void start(int argc, char** argv);

   private:
    std::shared_ptr<LuaEngine> m_Engine;
};
