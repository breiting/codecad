/**
 * @file EdgeSelector.cpp
 * @brief OCCT-backed implementation of ccad::select::EdgeSelector.
 */

#include "ccad/select/EdgeSelector.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_set>

// -------- internal OCCT bridge headers  --------
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <BRepLProp_CLProps.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomLProp_CLProps.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include "internal/MathHelper.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::select {

struct EdgeSelector::Impl {
    // ownership is shared through EdgeSelector copies
    TopoDS_Shape shape;
    double tol = 1e-6;

    // map edges -> index + attributes
    std::vector<EdgeRef> edges;          // index == position
    TopTools_IndexedMapOfShape edgeMap;  // to get back TopoDS_Edge if needed
    glm::vec3 bboxMin{}, bboxMax{};

    explicit Impl(const TopoDS_Shape& s, double tolerance) : shape(s), tol(tolerance) {
        buildCache();
    }

    void buildCache() {
        // 1) gather edges
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        edges.reserve(edgeMap.Extent());

        // bbox of full shape
        Bnd_Box bb;
        BRepBndLib::Add(shape, bb, false);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bb.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        bboxMin = {(float)xmin, (float)ymin, (float)zmin};
        bboxMax = {(float)xmax, (float)ymax, (float)zmax};

        // map edges -> adjacent faces (for dihedral)
        TopTools_IndexedDataMapOfShapeListOfShape edge2faces;
        TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edge2faces);

        // 2) compute attributes per edge
        edges.resize(edgeMap.Extent());

        for (int i = 1; i <= edgeMap.Extent(); ++i) {
            const TopoDS_Edge& E = TopoDS::Edge(edgeMap.FindKey(i));
            EdgeRef er;
            er.index = static_cast<size_t>(i - 1);

            // length
            GProp_GProps lp;
            BRepGProp::LinearProperties(E, lp);
            er.length = lp.Mass();

            BRepAdaptor_Curve c(E);
            const GeomAbs_CurveType ct = c.GetType();
            const Standard_Real f = c.FirstParameter();
            const Standard_Real l = c.LastParameter();
            const Standard_Real mid = 0.5 * (f + l);

            gp_Pnt Pm = c.Value(mid);
            gp_Dir Tg(1, 0, 0);

            try {
                // CL props über den Adaptor, nicht direkt über die Edge
                BRepLProp_CLProps lprop(c, /*DerivativeOrder*/ 1, /*Tol*/ tol);
                lprop.SetParameter(mid);

                // Punkt holen
                Pm = lprop.Value();

                // Tangente nur, wenn definiert
                if (lprop.IsTangentDefined()) {
                    gp_Dir t;
                    lprop.Tangent(t);  // schreibt in t
                    Tg = t;
                }
            } catch (...) {
                // Fallback: geometrische Mitte und Richtungsableitung vom Adaptor
                Pm = c.Value(mid);
                // Tg bleibt default (1,0,0)
            }

            er.anyPoint = ToGlm(Pm);
            er.approxDir = ToGlm(Tg);

            // Kreis/Arc → both are circular
            er.isCircular = (ct == GeomAbs_Circle);
            er.radius = er.isCircular ? c.Circle().Radius() : 0.0;

            // Dihedral
            er.dihedralDeg = 180.0;
            if (edge2faces.Contains(E)) {
                const TopTools_ListOfShape& lst = edge2faces.FindFromKey(E);
                if (lst.Extent() >= 2) {
                    TopTools_ListIteratorOfListOfShape it(lst);
                    const TopoDS_Face F1 = TopoDS::Face(it.Value());
                    it.Next();
                    const TopoDS_Face F2 = TopoDS::Face(it.Value());

                    BRepAdaptor_Surface s1(F1), s2(F2);
                    if (s1.GetType() == GeomAbs_Plane && s2.GetType() == GeomAbs_Plane) {
                        gp_Dir n1 = s1.Plane().Axis().Direction();
                        gp_Dir n2 = s2.Plane().Axis().Direction();
                        double a = CalculateAngleInDeg(n1, n2);
                        if (a > 180.0) a = 360.0 - a;
                        er.dihedralDeg = a;
                    }
                }
            }

            edges[er.index] = er;
        }
    }
};

// ---------- EdgeSet ops ----------
static std::unordered_set<size_t> indexSet(const EdgeSet& s) {
    std::unordered_set<size_t> out;
    out.reserve(s.size());
    for (auto& e : s.items()) out.insert(e.index);
    return out;
}

EdgeSet& EdgeSet::unite(const EdgeSet& b) {
    auto is = indexSet(*this);
    for (auto& e : b.m_items)
        if (is.insert(e.index).second) m_items.push_back(e);
    return *this;
}

EdgeSet& EdgeSet::intersect(const EdgeSet& b) {
    auto ib = indexSet(b);
    std::vector<EdgeRef> out;
    out.reserve(std::min(m_items.size(), b.m_items.size()));
    for (auto& e : m_items)
        if (ib.count(e.index)) out.push_back(e);
    m_items.swap(out);
    return *this;
}

EdgeSet& EdgeSet::subtract(const EdgeSet& b) {
    auto ib = indexSet(b);
    std::vector<EdgeRef> out;
    out.reserve(m_items.size());
    for (auto& e : m_items)
        if (!ib.count(e.index)) out.push_back(e);
    m_items.swap(out);
    return *this;
}

// ---------- EdgeSelector API ----------
EdgeSelector EdgeSelector::FromShape(const ccad::Shape& s, double tolerance) {
    auto occ = ShapeAsOcct(s);
    return EdgeSelector(std::make_shared<Impl>(occ->Occt(), tolerance));
}

EdgeSelector& EdgeSelector::geom(EdgeGeom g) {
    m_filters.geom = g;
    return *this;
}

EdgeSelector& EdgeSelector::lengthBetween(const LengthRange& r) {
    m_filters.length = r;
    return *this;
}

EdgeSelector& EdgeSelector::radiusBetween(const RadiusRange& r) {
    m_filters.radius = r;
    return *this;
}

EdgeSelector& EdgeSelector::dihedralBetween(const AngleRange& r) {
    m_filters.dihedral = r;
    return *this;
}

EdgeSelector& EdgeSelector::parallelTo(Axis axis, double tolDeg) {
    m_filters.parallelAxisTolDeg = std::make_pair(axis, tolDeg);
    return *this;
}

EdgeSelector& EdgeSelector::nearPlane(Axis axis, double value, double tol) {
    m_filters.planeAxisValTol = std::make_tuple(axis, value, tol);
    return *this;
}

EdgeSelector& EdgeSelector::onBoxSide(BoxSide side, double tol) {
    m_filters.boxSideTol = std::make_pair(side, tol);
    return *this;
}

EdgeSelector& EdgeSelector::insideAABB(const glm::vec3& minP, const glm::vec3& maxP) {
    m_filters.aabb = std::make_pair(minP, maxP);
    return *this;
}

EdgeSet EdgeSelector::collect() const {
    EdgeSet out;
    out.m_items.reserve(m_impl->edges.size());

    auto pass = [&](const EdgeRef& e) -> bool {
        // geom
        if (m_filters.geom) {
            switch (*m_filters.geom) {
                case EdgeGeom::Any:
                    break;
                case EdgeGeom::Line:
                    if (e.isCircular) return false;
                    break;
                case EdgeGeom::Circle:
                    if (!e.isCircular) return false;
                    // (note: true circles vs arcs both are "circular" here)
                    break;
                case EdgeGeom::Arc:
                    if (!e.isCircular) return false;
                    break;
            }
        }
        // length
        if (m_filters.length) {
            if (e.length < m_filters.length->min || e.length > m_filters.length->max) return false;
        }
        // radius
        if (m_filters.radius) {
            if (!e.isCircular) return false;
            if (e.radius < m_filters.radius->min || e.radius > m_filters.radius->max) return false;
        }
        // dihedral
        if (m_filters.dihedral) {
            if (e.dihedralDeg < m_filters.dihedral->minDeg || e.dihedralDeg > m_filters.dihedral->maxDeg) return false;
        }
        // parallel to axis
        if (m_filters.parallelAxisTolDeg) {
            auto [ax, tolDeg] = *m_filters.parallelAxisTolDeg;
            gp_Dir axis = GetAxisDirection(ax);
            gp_Dir d(std::max(1e-12f, std::abs(e.approxDir.x)) * (e.approxDir.x >= 0 ? 1 : -1),
                     std::max(1e-12f, std::abs(e.approxDir.y)) * (e.approxDir.y >= 0 ? 1 : -1),
                     std::max(1e-12f, std::abs(e.approxDir.z)) * (e.approxDir.z >= 0 ? 1 : -1));
            double a = CalculateAngleInDeg(axis, d);
            a = std::min(a, 180.0 - a);  // accept both directions
            if (a > tolDeg) return false;
        }
        // near plane axis=value
        if (m_filters.planeAxisValTol) {
            auto [ax, val, tol] = *m_filters.planeAxisValTol;
            double coord = 0.0;
            switch (ax) {
                case Axis::X:
                    coord = e.anyPoint.x;
                    break;
                case Axis::Y:
                    coord = e.anyPoint.y;
                    break;
                case Axis::Z:
                    coord = e.anyPoint.z;
                    break;
            }
            if (std::abs(coord - val) > tol) return false;
        }
        // on box side
        if (m_filters.boxSideTol) {
            auto [side, tol] = *m_filters.boxSideTol;
            switch (side) {
                case BoxSide::XMin:
                    if (std::abs(e.anyPoint.x - m_impl->bboxMin.x) > tol) return false;
                    break;
                case BoxSide::XMax:
                    if (std::abs(e.anyPoint.x - m_impl->bboxMax.x) > tol) return false;
                    break;
                case BoxSide::YMin:
                    if (std::abs(e.anyPoint.y - m_impl->bboxMin.y) > tol) return false;
                    break;
                case BoxSide::YMax:
                    if (std::abs(e.anyPoint.y - m_impl->bboxMax.y) > tol) return false;
                    break;
                case BoxSide::ZMin:
                    if (std::abs(e.anyPoint.z - m_impl->bboxMin.z) > tol) return false;
                    break;
                case BoxSide::ZMax:
                    if (std::abs(e.anyPoint.z - m_impl->bboxMax.z) > tol) return false;
                    break;
            }
        }
        // inside AABB
        if (m_filters.aabb) {
            const auto& minP = m_filters.aabb->first;
            const auto& maxP = m_filters.aabb->second;
            const glm::vec3& p = e.anyPoint;
            if (p.x < minP.x || p.y < minP.y || p.z < minP.z) return false;
            if (p.x > maxP.x || p.y > maxP.y || p.z > maxP.z) return false;
        }

        return true;
    };

    for (const auto& e : m_impl->edges) {
        if (pass(e)) out.m_items.push_back(e);
    }
    return out;
}

}  // namespace ccad::select
