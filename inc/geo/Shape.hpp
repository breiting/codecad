/// \file
/// Geometry API for solid modeling (OCCT wrappers).
#pragma once
#include <TopoDS_Shape.hxx>
#include <memory>

namespace geo {

class Shape {
   public:
    explicit Shape(const TopoDS_Shape& s) : m_Shape(s) {
    }
    const TopoDS_Shape& Get() const {
        return m_Shape;
    }

   private:
    TopoDS_Shape m_Shape;
};

using ShapePtr = std::shared_ptr<Shape>;

}  // namespace geo
