#pragma once
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <Standard_TypeDef.hxx>
#include <TopoDS_Shape.hxx>
#include <memory>

#include "ccad/geom/IShape.hpp"
#include "ccad/geom/Math.hpp"

namespace ccad {

/** \brief Concrete IShape implementation that owns an OCCT TopoDS_Shape. */
class OcctShape final : public IShape {
   public:
    explicit OcctShape(const TopoDS_Shape& s) : m_Shape(s) {
    }
    explicit OcctShape(TopoDS_Shape&& s) : m_Shape(std::move(s)) {
    }

    std::string TypeName() const override {
        return "OcctShape";
    }

    Bounds BoundingBox() const override {
        Bnd_Box bb;
        BRepBndLib::Add(m_Shape, bb);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        return Bounds{{xmin, ymin, zmin}, {xmax, ymax, zmax}};
    }

    std::unique_ptr<IShape> Clone() const override {
        return std::make_unique<OcctShape>(m_Shape);  // TopoDS_Shape is a handle (shared) – OK for clone semantics
    }

    const TopoDS_Shape& Occt() const {
        return m_Shape;
    }
    TopoDS_Shape& Occt() {
        return m_Shape;
    }

   private:
    TopoDS_Shape m_Shape;
};

}  // namespace ccad
