#pragma once

#include <memory>
#include <string>

class Viewer {
   public:
    virtual ~Viewer() = default;
    virtual void start(const std::string& projectFile) = 0;

    static std::unique_ptr<Viewer> create();
};
