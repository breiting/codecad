#pragma once

#include <core/Viewer.hpp>

class CosmaViewer : Viewer {
   public:
    void start(const std::string& projectFile) override;
};
