#pragma once

#include <string>
#include <vector>

#include "sol/state.hpp"

struct BomItem {
    std::string partId;
    std::string name;
    int qty;
    std::string material;
    std::string category;
    double width;
    double depth;
    double height;
    double diameter;
    double thickness;
    std::string notes;
};

namespace io {
class BomWriter {
   public:
    BomWriter() = default;

    void Collect(sol::state& lua, const std::string& partId);

    void WriteCsv(const std::string& fileName);
    void WriteMarkdown(const std::string& fileName);

   private:
    std::vector<BomItem> m_BomItem;
};
}  // namespace io
