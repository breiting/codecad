#include "io/Bom.hpp"

#include <fstream>

namespace io {

void BomWriter::Collect(sol::state& lua, const std::string& partId) {
    sol::object mod = lua["require"]("ccad.util.bom");
    sol::table items = mod.as<sol::table>()["items"]().get<sol::table>();
    for (auto& kv : items) {
        sol::table it = kv.second.as<sol::table>();
        BomItem bi;
        bi.partId = partId;
        bi.name = it.get_or("name", std::string("Item"));
        bi.qty = it.get_or("qty", 1);
        bi.material = it.get_or("material", std::string());
        bi.category = it.get_or("category", std::string());
        bi.width = it.get_or("w", 0.0);
        bi.depth = it.get_or("d", 0.0);
        bi.height = it.get_or("h", 0.0);
        bi.diameter = it.get_or("diameter", 0.0);
        bi.thickness = it.get_or("thickness", 0.0);
        bi.notes = it.get_or("notes", std::string());
        m_BomItem.emplace_back(std::move(bi));
    }
}

void BomWriter::WriteCsv(const std::string& fileName) {
    std::ofstream os(fileName);
    os << "part;name;qty;material;category;w_mm;d_mm;h_mm;diameter_mm;thickness_mm;notes\n";
    for (auto& it : m_BomItem) {
        auto esc = [](const std::string& s) {
            if (s.find_first_of(";\n\"") != std::string::npos) {
                std::string t = s;
                size_t pos{};
                while ((pos = t.find('"', pos)) != std::string::npos) {
                    t.replace(pos, 1, "\"\"");
                    pos += 2;
                }
                return "\"" + t + "\"";
            }
            return s;
        };
        os << esc(it.partId) << ';' << esc(it.name) << ';' << it.qty << ';' << esc(it.material) << ';'
           << esc(it.category) << ';' << (it.width ? std::to_string((int)std::round(it.width)) : "") << ';'
           << (it.depth ? std::to_string((int)std::round(it.depth)) : "") << ';'
           << (it.height ? std::to_string((int)std::round(it.height)) : "") << ';'
           << (it.diameter ? std::to_string((int)std::round(it.diameter)) : "") << ';'
           << (it.thickness ? std::to_string((int)std::round(it.thickness)) : "") << ';' << esc(it.notes) << '\n';
    }
}

void BomWriter::WriteMarkdown(const std::string& fileName) {
    std::ofstream os(fileName);

    os << "| Part | Name | Qty | Material | Category | W (mm) | D (mm) | H (mm) | Ø (mm) | t (mm) | Notes |\n";
    os << "|---|---|---:|---|---|---:|---:|---:|---:|---:|---|\n";
    auto fmt = [](double v) { return v == 0 ? std::string("") : std::to_string((int)std::round(v)); };
    for (auto& it : m_BomItem) {
        os << "| " << it.partId << " | " << it.name << " | " << it.qty << " | " << it.material << " | " << it.category
           << " | " << fmt(it.width) << " | " << fmt(it.depth) << " | " << fmt(it.height) << " | " << fmt(it.diameter)
           << " | " << fmt(it.thickness) << " | " << it.notes << " |\n";
    }
}

}  // namespace io
