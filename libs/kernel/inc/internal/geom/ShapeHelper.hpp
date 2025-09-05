#pragma once
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <memory>

#include "ccad/base/IShape.hpp"
#include "ccad/base/Shape.hpp"
#include "internal/geom/OcctShape.hpp"

namespace ccad {
inline const OcctShape* ShapeAsOcct(const Shape& s) {
    return dynamic_cast<const OcctShape*>(&const_cast<IShape&>(s.Get()));
}

inline Shape WrapOcctShape(const TopoDS_Shape& s) {
    return Shape{std::make_unique<OcctShape>(std::move(s))};
}

inline bool IsValid(const TopoDS_Shape& s) {
    BRepCheck_Analyzer ana(s);
    return ana.IsValid();
}

inline TopoDS_Shape FixIfNeeded(const TopoDS_Shape& s) {
    if (IsValid(s)) return s;
    Handle(ShapeFix_Shape) fixer = new ShapeFix_Shape(s);
    fixer->Perform();
    return fixer->Shape();
}

}  // namespace ccad
