#pragma once
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
}  // namespace ccad
