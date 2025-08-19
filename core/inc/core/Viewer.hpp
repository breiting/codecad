#pragma once

#include <filesystem>
#include <string>
class Viewer {
   public:
    virtual ~Viewer() = default;
    virtual void start(const std::string& projectFile, std::filesystem::path& outDir) = 0;
};
