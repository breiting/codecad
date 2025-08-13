#include "geo/Sketch.hpp"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt.hxx>
#include <vector>

namespace {

static TopoDS_Wire MakeWireFromXY(const std::vector<std::pair<double, double>>& pts, bool closed) {
    BRepBuilderAPI_MakeWire mw;
    if (pts.size() < 2) return mw.Wire();
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        const auto& a = pts[i];
        const auto& b = pts[i + 1];
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(a.first, a.second, 0.0), gp_Pnt(b.first, b.second, 0.0)));
    }
    if (closed) {
        const auto& a = pts.back();
        const auto& b = pts.front();
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(a.first, a.second, 0.0), gp_Pnt(b.first, b.second, 0.0)));
    }
    return mw.Wire();
}

static TopoDS_Wire MakeWireFromXZ(const std::vector<std::pair<double, double>>& rz, bool closed) {
    BRepBuilderAPI_MakeWire mw;
    if (rz.size() < 2) return mw.Wire();
    for (size_t i = 0; i + 1 < rz.size(); ++i) {
        const auto& a = rz[i];
        const auto& b = rz[i + 1];
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(a.first, 0.0, a.second), gp_Pnt(b.first, 0.0, b.second)));
    }
    if (closed) {
        const auto& a = rz.back();
        const auto& b = rz.front();
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(a.first, 0.0, a.second), gp_Pnt(b.first, 0.0, b.second)));
    }
    return mw.Wire();
}

}  // namespace

namespace geo {

ShapePtr PolylineXY_Face(const std::vector<std::pair<double, double>>& pts, bool closed) {
    TopoDS_Wire w = MakeWireFromXY(pts, closed);
    if (w.IsNull()) return nullptr;
    TopoDS_Face f = BRepBuilderAPI_MakeFace(w).Face();
    return std::make_shared<Shape>(f);
}

ShapePtr PolylineXZ_Face(const std::vector<std::pair<double, double>>& rz, bool closed, bool close_to_axis) {
    TopoDS_Wire w = MakeWireFromXZ(rz, closed);
    if (w.IsNull()) return nullptr;
    if (close_to_axis) {
        // schließe zur Z-Achse (r=0): letzter Punkt → (0,z_last) → (0,z_first) → erster Punkt
        BRepBuilderAPI_MakeWire mw(w);
        const auto& p0 = rz.front();
        const auto& pn = rz.back();
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(pn.first, 0.0, pn.second), gp_Pnt(0.0, 0.0, pn.second)));
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, 0.0, pn.second), gp_Pnt(0.0, 0.0, p0.second)));
        mw.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, 0.0, p0.second), gp_Pnt(p0.first, 0.0, p0.second)));
        w = mw.Wire();
    }
    TopoDS_Face f = BRepBuilderAPI_MakeFace(w).Face();
    return std::make_shared<Shape>(f);
}

}  // namespace geo
