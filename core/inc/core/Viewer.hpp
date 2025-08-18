#pragma once

#include <string>
class Viewer {
   public:
    virtual ~Viewer() = default;
    virtual void start(const std::string& projectFile) = 0;
};
