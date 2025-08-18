#pragma once

#include <core/Viewer.hpp>

class CosmaViewer : public Viewer {
   public:
    CosmaViewer();
    void start(const std::string& projectFile) override;
};
