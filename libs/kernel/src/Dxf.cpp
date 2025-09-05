#include <ccad/base/Exception.hpp>
#include <ccad/io/Dxf.hpp>

namespace ccad::io {

DxfWriter::DxfWriter(const std::string &fileName) {
    m_File.open(fileName);
    if (!m_File) throw Exception("Cannot open DXF file for writing");
    WriteHeader();
}

DxfWriter::~DxfWriter() {
    Flush();
}

void DxfWriter::Flush() {
    if (m_File.is_open()) {
        WriteFooter();
        m_File.close();
    }
}

void DxfWriter::AddPolyline(const Polyline2D &polyline, bool closed) {
    if (polyline.points.empty()) return;

    m_File << "0\nLWPOLYLINE\n";
    m_File << "8\n0\n";                                  // Layer 0
    m_File << "90\n" << polyline.points.size() << "\n";  // number of vertices
    m_File << "70\n" << (closed ? 1 : 0) << "\n";        // 1 = closed polyline

    for (auto &p : polyline.points) {
        m_File << "10\n" << p.x << "\n";
        m_File << "20\n" << p.y << "\n";
    }
}

void DxfWriter::WriteHeader() {
    m_File << "0\nSECTION\n2\nHEADER\n0\nENDSEC\n";
    m_File << "0\nSECTION\n2\nTABLES\n0\nENDSEC\n";
    m_File << "0\nSECTION\n2\nBLOCKS\n0\nENDSEC\n";
    m_File << "0\nSECTION\n2\nENTITIES\n";
}

void DxfWriter::WriteFooter() {
    m_File << "0\nENDSEC\n";
    m_File << "0\nEOF\n";
}

}  // namespace ccad::io
