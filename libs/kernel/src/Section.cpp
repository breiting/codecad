#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <Geom_Plane.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <ccad/base/Logger.hpp>
#include <ccad/base/Math.hpp>
#include <gp_Pln.hxx>

#include "ccad/base/Shape.hpp"
#include "internal/geom/OcctShape.hpp"
#include "internal/geom/ShapeHelper.hpp"

namespace ccad::draft {

static inline Handle(Geom_Plane) MakeOcctPlane(const Plane3& p) {
    return new Geom_Plane(gp_Pln(gp_Pnt(p.point.x, p.point.y, p.point.z), gp_Dir(p.normal.x, p.normal.y, p.normal.z)));
}

// map a 3D point to 2D coordinates in arbitrary plane
static inline Vec2 MapTo2D(const Geom_Plane* plane, const gp_Pnt& p) {
    // Koordinatensystem der Ebene (Ax3 = Ursprung + Achsen)
    gp_Ax3 ax3 = plane->Position();

    gp_Pnt origin = ax3.Location();
    gp_Dir xDir = ax3.XDirection();  // "U-Achse"
    gp_Dir yDir = ax3.YDirection();  // "V-Achse"

    gp_Vec vec(origin, p);

    double u = vec.Dot(gp_Vec(xDir));
    double v = vec.Dot(gp_Vec(yDir));

    return Vec2(u, v);
}

std::vector<Polyline2D> SectionWithPlane(const Shape& shape, const Plane3& planeIn, double defl) {
    auto os = ShapeAsOcct(shape);
    auto s = os->Occt();
    if (!os) throw std::runtime_error("SectionOutline: non-OCCT shape implementation");

    std::vector<Polyline2D> out;

    Handle(Geom_Plane) plane = MakeOcctPlane(planeIn);

    try {
        BRepAlgoAPI_Section sec(s, plane, false);
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
            pl.points.reserve(static_cast<size_t>(disc.NbPoints()));
            for (int i = 1; i <= disc.NbPoints(); ++i) {
                gp_Pnt p = disc.Value(i);
                pl.points.emplace_back(MapTo2D(plane.get(), p));
            }
            out.emplace_back(std::move(pl));
        }
    } catch (const Standard_Failure& e) {
        LOG(ERROR) << "[SectionOutline2D] OpenCascade error: " << e.GetMessageString() << "\n";
    } catch (...) {
        LOG(ERROR) << "[SectionOutline2D] unknown error.\n";
    }
    return out;
}

}  // namespace ccad::draft
