#pragma once

#include "io/Project.hpp"

class Viewer {
   public:
    virtual ~Viewer() = default;
    virtual void start(const Project &project) = 0;
};
