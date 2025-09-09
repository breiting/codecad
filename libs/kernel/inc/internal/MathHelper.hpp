#pragma once
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS_Shape.hxx>
#include <ccad/base/Math.hpp>
#include <glm/glm.hpp>

namespace ccad {

inline double CalculateAngleInDeg(const gp_Dir& a, const gp_Dir& b) {
    double dot = ClampToUnit(a.X() * b.X() + a.Y() * b.Y() + a.Z() * b.Z());
    return DegToRad(std::acos(dot));
}

inline glm::vec3 ToGlm(const gp_Dir& d) {
    return {(float)d.X(), (float)d.Y(), (float)d.Z()};
}

inline glm::vec3 ToGlm(const gp_Pnt& p) {
    return {(float)p.X(), (float)p.Y(), (float)p.Z()};
}

inline gp_Dir GetAxisDirection(Axis ax) {
    switch (ax) {
        case Axis::X:
            return gp_Dir(1, 0, 0);
        case Axis::Y:
            return gp_Dir(0, 1, 0);
        case Axis::Z:
        default:
            return gp_Dir(0, 0, 1);
    }
}

}  // namespace ccad
