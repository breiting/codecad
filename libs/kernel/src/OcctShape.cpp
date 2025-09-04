#include "private/geom/OcctShape.hpp"

#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <Standard_TypeDef.hxx>

#include "ccad/geom/Math.hpp"

using namespace ccad;

Bounds ccad::OcctShape::BoundingBox() const {
    Bnd_Box bb;
    BRepBndLib::Add(m_Shape, bb);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    return Bounds{{xmin, ymin, zmin}, {xmax, ymax, zmax}};
}
