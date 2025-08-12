#include "io/Manifest.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "io/Export.hpp"

namespace fs = std::filesystem;

static std::string JsonEscape(const std::string& s) {
    std::ostringstream o;
    for (auto c : s) {
        switch (c) {
            case '\"':
                o << "\\\"";
                break;
            case '\\':
                o << "\\\\";
                break;
            case '\n':
                o << "\\n";
                break;
            case '\r':
                o << "\\r";
                break;
            case '\t':
                o << "\\t";
                break;
            default:
                o << c;
                break;
        }
    }
    return o.str();
}

namespace io {

bool WriteAssemblyManifest(const runtime::Assembly& A, const std::string& outdir, bool write_step) {
    try {
        fs::create_directories(outdir);
        // Dateien schreiben + JSON zusammenbauen
        std::ostringstream json;
        json << "{\n";
        json << "  \"name\": \"" << JsonEscape(A.name) << "\",\n";
        json << "  \"parts\": [\n";
        for (size_t i = 0; i < A.parts.size(); ++i) {
            const auto& p = A.parts[i];
            const std::string base = A.name + "__" + p.name;
            const auto stl = fs::path(outdir) / (base + ".stl");
            const auto step = fs::path(outdir) / (base + ".step");

            io::SaveSTL(p.shape, stl.string(), 0.1);
            if (write_step) io::SaveSTEP(p.shape, step.string());

            json << "    {\"name\":\"" << JsonEscape(p.name) << "\","
                 << "\"stl\":\"" << JsonEscape(stl.filename().string()) << "\","
                 << "\"step\":\"" << JsonEscape(step.filename().string()) << "\","
                 << "\"explode\":[" << p.ex << "," << p.ey << "," << p.ez << "]"
                 << "}";
            if (i + 1 < A.parts.size()) json << ",";
            json << "\n";
        }
        json << "  ]\n";
        json << "}\n";

        std::ofstream mf(fs::path(outdir) / (A.name + "__manifest.json"));
        mf << json.str();
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace io
