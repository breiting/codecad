#pragma once

#include <core/CoreEngine.hpp>
#include <core/Viewer.hpp>

class CosmaViewer : public Viewer {
   public:
    CosmaViewer();
    void start(const std::string& projectFile, std::unique_ptr<CoreEngine> engine) override;
};
