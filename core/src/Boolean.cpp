#include "geometry/Boolean.hpp"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>

namespace geometry {

ShapePtr MakeUnion(const std::vector<ShapePtr>& shapes) {
    if (shapes.size() < 2) return nullptr;
    TopoDS_Shape acc = shapes[0]->Get();
    for (size_t i = 1; i < shapes.size(); ++i) {
        acc = BRepAlgoAPI_Fuse(acc, shapes[i]->Get()).Shape();
    }
    return std::make_shared<Shape>(acc);
}

ShapePtr MakeDifference(const ShapePtr& a, const ShapePtr& b) {
    if (!a || !b) return nullptr;
    TopoDS_Shape r = BRepAlgoAPI_Cut(a->Get(), b->Get()).Shape();
    return std::make_shared<Shape>(r);
}

ShapePtr MakeIntersect(const ShapePtr& a, const ShapePtr& b) {
    if (!a || !b) return nullptr;
    TopoDS_Shape r = BRepAlgoAPI_Common(a->Get(), b->Get()).Shape();
    return std::make_shared<Shape>(r);
}

}  // namespace geometry
