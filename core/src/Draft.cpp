#include "geo/Draft.hpp"

#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Geom_Plane.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pln.hxx>
#include <stdexcept>

namespace geo {

static inline Handle(Geom_Plane) MakePlane(char axis, double v) {
    switch (axis) {
        case 'x':
        case 'X':
            return new Geom_Plane(gp_Pln(gp_Pnt(v, 0, 0), gp_Dir(1, 0, 0)));
        case 'y':
        case 'Y':
            return new Geom_Plane(gp_Pln(gp_Pnt(0, v, 0), gp_Dir(0, 1, 0)));
        case 'z':
        case 'Z':
            return new Geom_Plane(gp_Pln(gp_Pnt(0, 0, v), gp_Dir(0, 0, 1)));
        default:
            throw std::invalid_argument("SectionOutline2D: axis must be x, y, or z");
    }
}

// map a 3D point to 2D plane coordinates (see header doc)
static inline std::pair<double, double> MapTo2D(char axis, const gp_Pnt& p) {
    switch (axis) {
        case 'x':
        case 'X':
            return {p.Y(), p.Z()};
        case 'y':
        case 'Y':
            return {p.X(), p.Z()};
        case 'z':
        case 'Z':
            return {p.X(), p.Y()};
        default:
            return {0.0, 0.0};
    }
}

std::vector<Polyline2D> SectionOutline2D(const ShapePtr& s, char axis, double value, double defl) {
    std::vector<Polyline2D> out;
    if (!s || s->Get().IsNull()) return out;

    Handle(Geom_Plane) plane = MakePlane(axis, value);

    try {
        BRepAlgoAPI_Section sec(s->Get(), plane, false);
        sec.Approximation(true);
        sec.Build();
        if (!sec.IsDone()) return out;

        for (TopExp_Explorer ex(sec.Shape(), TopAbs_EDGE); ex.More(); ex.Next()) {
            const TopoDS_Edge E = TopoDS::Edge(ex.Current());
            if (E.IsNull()) continue;

            BRepAdaptor_Curve ac(E);
            // discretize with uniform deflection (works for lines too)
            GCPnts_UniformDeflection disc(ac, defl);
            if (!disc.IsDone() || disc.NbPoints() < 2) continue;

            Polyline2D pl;
            pl.reserve(static_cast<size_t>(disc.NbPoints()));
            for (int i = 1; i <= disc.NbPoints(); ++i) {
                gp_Pnt p = disc.Value(i);
                pl.emplace_back(MapTo2D(axis, p));
            }
            out.emplace_back(std::move(pl));
        }
    } catch (const Standard_Failure& e) {
        std::cerr << "[SectionOutline2D] OpenCascade error: " << e.GetMessageString() << "\n";
    } catch (...) {
        std::cerr << "[SectionOutline2D] unknown error.\n";
    }
    return out;
}

}  // namespace geo
