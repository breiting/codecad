#include <BRepAlgoAPI_Fuse.hxx>

#include "Geo/Boolean.hpp"

namespace Geo {

ShapePtr MakeUnion(const std::vector<ShapePtr>& shapes) {
    if (shapes.size() < 2) return nullptr;
    TopoDS_Shape acc = shapes[0]->Get();
    for (size_t i = 1; i < shapes.size(); ++i) {
        acc = BRepAlgoAPI_Fuse(acc, shapes[i]->Get()).Shape();
    }
    return std::make_shared<Shape>(acc);
}

}  // namespace Geo
