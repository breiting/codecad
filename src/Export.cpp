#include "io/Export.hpp"

#include <BRepMesh_IncrementalMesh.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <iostream>

namespace io {

bool SaveSTL(const geo::ShapePtr& shape, const std::string& path, double deflection) {
    if (!shape) return false;
    // Tessellate
    BRepMesh_IncrementalMesh mesher(shape->Get(), deflection);
    mesher.Perform();
    StlAPI_Writer writer;
    if (writer.Write(shape->Get(), path.c_str())) {
        std::cout << "Wrote STL: " << path << "\n";
        return true;
    }
    std::cerr << "Failed to write STL: " << path << "\n";
    return false;
}

bool SaveSTEP(const geo::ShapePtr& shape, const std::string& path) {
    if (!shape) return false;
    STEPControl_Writer stepWriter;
    IFSelect_ReturnStatus status = stepWriter.Transfer(shape->Get(), STEPControl_AsIs);
    if (status != IFSelect_RetDone) {
        std::cerr << "STEP transfer failed\n";
        return false;
    }
    if (stepWriter.Write(path.c_str()) == IFSelect_RetDone) {
        std::cout << "Wrote STEP: " << path << "\n";
        return true;
    }
    std::cerr << "Failed to write STEP: " << path << "\n";
    return false;
}

}  // namespace io
