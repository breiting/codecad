#pragma once

/// This interface can be used run without the cosma viewer

#include <string>

class CoreEngine;

class Viewer {
   public:
    virtual ~Viewer() = default;
    virtual void start(const std::string& projectFile, std::unique_ptr<CoreEngine> engine) = 0;
};
