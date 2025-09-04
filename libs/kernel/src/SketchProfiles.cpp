#include <ccad/base/Exception.hpp>
#include <ccad/sketch/SketchProfiles.hpp>

// OCCT
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS.hxx>
#include <cmath>
#include <gp_Pnt.hxx>

#include "internal/geom/ShapeHelper.hpp"

namespace ccad::sketch {

namespace {
constexpr double kEps = 1e-9;

static bool almostEqual(double a, double b, double eps = kEps) {
    return std::abs(a - b) <= eps;
}

static bool same2(const Vec2& a, const Vec2& b) {
    return almostEqual(a.x, b.x) && almostEqual(a.y, b.y);
}

// Remove strictly consecutive duplicates
static std::vector<Vec2> dedupeConsecutive(const std::vector<Vec2>& in) {
    std::vector<Vec2> out;
    out.reserve(in.size());
    for (const auto& p : in) {
        if (out.empty() || !same2(out.back(), p)) {
            out.push_back(p);
        }
    }
    return out;
}

// Make edges from a list of 3D points (open poly)
static void addPolylineEdges(BRepBuilderAPI_MakeWire& w, const std::vector<gp_Pnt>& pts) {
    for (std::size_t i = 1; i < pts.size(); ++i) {
        if (!pts[i].IsEqual(pts[i - 1], kEps)) w.Add(BRepBuilderAPI_MakeEdge(pts[i - 1], pts[i]));
    }
}

}  // namespace

Shape PolyXY(const std::vector<Vec2>& ptsIn) {
    auto pts = dedupeConsecutive(ptsIn);
    if (pts.size() < 3) {
        throw Exception("PolyXY: need at least 3 distinct points");
    }

    // Build closed wire in XY (Z=0)
    BRepBuilderAPI_MakeWire wire;
    for (std::size_t i = 0; i < pts.size(); ++i) {
        const auto& a = pts[i];
        const auto& b = pts[(i + 1) % pts.size()];
        if (!same2(a, b)) {
            wire.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(a.x, a.y, 0.0), gp_Pnt(b.x, b.y, 0.0)));
        }
    }
    if (!wire.IsDone()) {
        throw Exception("PolyXY: failed to build wire");
    }

    // Create face from wire
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire.Wire());
    return WrapOcctShape(face);
}

Shape ProfileXZ(const std::vector<Vec2>& ptsIn, bool closed, bool closeToAxis) {
    auto pts = dedupeConsecutive(ptsIn);
    if (pts.size() < 2) {
        throw Exception("ProfileXZ: need at least 2 distinct points");
    }

    // Prepare 3D points in XZ plane (Y=0)
    std::vector<gp_Pnt> p3;
    p3.reserve(pts.size() + (closeToAxis ? 3 : (closed ? 1 : 0)));

    for (const auto& p : pts) {
        p3.emplace_back(p.x, 0.0, p.y);
    }

    if (closeToAxis) {
        // (lastX,lastZ) -> (0,lastZ) -> (0,firstZ) -> (firstX,firstZ)
        const Vec2& first = pts.front();
        const Vec2& last = pts.back();

        // Append axis-closure points
        if (!almostEqual(last.x, 0.0)) {
            p3.emplace_back(0.0, 0.0, last.y);
        }
        if (!almostEqual(last.y, first.y)) {
            p3.emplace_back(0.0, 0.0, first.y);
        }
        if (!almostEqual(first.x, p3.back().X())) {
            p3.emplace_back(first.x, 0.0, first.y);
        }
        // This produces a closed contour suitable for revolve
    } else if (closed) {
        // Explicitly close wire back to first
        const auto& first = p3.front();
        const auto& last = p3.back();
        if (!last.IsEqual(first, kEps)) {
            p3.push_back(first);
        }
    }

    if (p3.size() < 2) {
        throw Exception("ProfileXZ: degenerate after closure");
    }

    BRepBuilderAPI_MakeWire wire;
    addPolylineEdges(wire, p3);

    if (!wire.IsDone()) {
        throw Exception("ProfileXZ: failed to build wire");
    }

    // Return wire as Shape (not a face!), RevolveZ will consume this.
    return WrapOcctShape(wire.Wire());
}

}  // namespace ccad::sketch
