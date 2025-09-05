#pragma once

#include <ccad/base/Math.hpp>
#include <fstream>

/**
 * \brief A very minimalistic DXF writer, mainly for debugging purposes. In future it would make sense to
 * add the libdxf library and replace this code!
 * */

namespace ccad {
namespace io {

class DxfWriter {
   public:
    explicit DxfWriter(const std::string &fileName);
    virtual ~DxfWriter();

    void Flush();

    void AddPolyline(const Polyline2D &polyline, bool closed = false);

   private:
    void WriteHeader();
    void WriteFooter();

   private:
    std::ofstream m_File;
};

}  // namespace io
}  // namespace ccad
