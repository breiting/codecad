#include <BRepMesh_IncrementalMesh.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <ccad/base/Logger.hpp>
#include <ccad/io/Export.hpp>

#include "internal/geom/ShapeHelper.hpp"

namespace ccad::io {

bool SaveSTL(const Shape& shape, const std::string& path, geom::TriangulationParams p) {
    auto s = ShapeAsOcct(shape);
    if (!s) throw std::runtime_error("SaveSTL: non-OCCT shape implementation");

    auto os = s->Occt();

    // TODO: maybe check if already triangulated, and use the cached one!

    BRepMesh_IncrementalMesh mesher(os, p.linearDeflection, false, DegToRad(p.angularDeflectionDeg), p.parallel);
    mesher.Perform();
    StlAPI_Writer writer;
    if (writer.Write(os, path.c_str())) {
        LOG(INFO) << "Wrote STL: " << path << "\n";
        return true;
    }
    LOG(ERROR) << "Failed to write STL: " << path << "\n";
    return false;
}

bool SaveSTEP(const Shape& shape, const std::string& path) {
    auto s = ShapeAsOcct(shape);
    if (!s) throw std::runtime_error("SaveSTL: non-OCCT shape implementation");

    auto os = s->Occt();

    STEPControl_Writer stepWriter;
    IFSelect_ReturnStatus status = stepWriter.Transfer(os, STEPControl_AsIs);
    if (status != IFSelect_RetDone) {
        LOG(ERROR) << "STEP transfer failed\n";
        return false;
    }
    if (stepWriter.Write(path.c_str()) == IFSelect_RetDone) {
        LOG(INFO) << "Wrote STEP: " << path << "\n";
        return true;
    }
    LOG(ERROR) << "Failed to write STEP: " << path << "\n";
    return false;
}

}  // namespace ccad::io
