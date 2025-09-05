#include "ccad/mech/PipeAdapter.hpp"

#include <algorithm>

// OCCT
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

#include "ccad/base/Exception.hpp"
#include "ccad/base/Status.hpp"
#include "internal/geom/ShapeHelper.hpp"

using std::max;

namespace ccad {
namespace mech {

static inline double Blend(double t, const PipeBlendOpts& o) {
    // map t∈[0,1] to s∈[0,1] with chosen blend
    t = Clamp(t, 0.0, 1.0);

    switch (o.kind) {
        case PipeBlendOpts::Kind::Tanh: {
            // s = 0.5 * (1 + tanh(k*(t-0.5)))   (k ~ 4..12)
            const double k = max(0.1, o.steepness);
            return 0.5 * (1.0 + std::tanh(k * (t - 0.5)));
        }
        case PipeBlendOpts::Kind::Logistic: {
            // s = 1 / (1 + exp(-k*(t-0.5)))
            const double k = max(0.1, o.steepness);
            return 1.0 / (1.0 + std::exp(-k * (t - 0.5)));
        }
        case PipeBlendOpts::Kind::Smoothstep: {
            // cubic smoothstep 3t^2 - 2t^3
            return t * t * (3.0 - 2.0 * t);
        }
    }
    return t;
}

static inline void Sanitize(PipeEnds& e, const PipeBlendOpts& o) {
    if (e.length <= 0.0) {
        throw Exception("PipeAdapter: Length must be > 0", Status::ERROR_OCCT);
    }
    auto fix_pair = [&](double& rin, double& rout) {
        rin = max(0.0, rin);
        rout = max(rin + o.minWall, rout);
    };
    fix_pair(e.rIn0, e.rOut0);
    fix_pair(e.rIn1, e.rOut1);
}

Shape PipeAdapter(const PipeEnds& endsIn, const PipeBlendOpts& opts) {
    PipeEnds e = endsIn;
    Sanitize(e, opts);

    const int N = max(8, opts.samples);
    const double L = e.length;
    const double dz = L / static_cast<double>(N - 1);
    const double eps = max(1e-6, opts.epsilon);

    // Sample OUTER and INNER curves as XZ points (y=0), X = radius(z), Z = z
    // Outer is traced forward z=0..L; Inner is traced backward L..0 (to close face CCW).
    TColgp_Array1OfPnt outPts(1, N);
    TColgp_Array1OfPnt inPts(1, N);

    for (int i = 0; i < N; ++i) {
        const double z = i * dz;
        const double t = (N == 1) ? 0.0 : (static_cast<double>(i) / (N - 1));
        const double s = Blend(t, opts);

        const double rOut = max(0.0, Lerp(e.rOut0, e.rOut1, s));
        const double rIn = Clamp(Lerp(e.rIn0, e.rIn1, s), 0.0, rOut - opts.minWall);

        outPts.SetValue(i + 1, gp_Pnt(rOut, 0.0, z));
        // inner curve sampled forward; but we will reverse order for the wire
        inPts.SetValue(i + 1, gp_Pnt(rIn, 0.0, z));
    }

    // Convert samples to C2 BSplines
    GeomAPI_PointsToBSpline outBuilder(outPts, 3, 8, GeomAbs_C2, 1.0e-6);
    GeomAPI_PointsToBSpline inBuilder(inPts, 3, 8, GeomAbs_C2, 1.0e-6);

    if (!outBuilder.IsDone() || !inBuilder.IsDone()) {
        throw std::runtime_error("PipeTransition: BSpline fit failed");
    }

    Handle(Geom_BSplineCurve) outC = outBuilder.Curve();
    Handle(Geom_BSplineCurve) inC = inBuilder.Curve();

    // Edges for outer/inner curves
    TopoDS_Edge eOut = BRepBuilderAPI_MakeEdge(outC);
    TopoDS_Edge eIn = BRepBuilderAPI_MakeEdge(inC);

    // Link edges to a closed wire:
    //   outer (0->L)
    //   top   (rOut(L) -> rIn(L))
    //   inner (L->0)   [reverse]
    //   bottom(rIn(0) -> rOut(0))
    gp_Pnt out0 = outPts.Value(1);
    gp_Pnt outL = outPts.Value(N);
    gp_Pnt in0 = inPts.Value(1);
    gp_Pnt inL = inPts.Value(N);

    TopoDS_Edge eTop = BRepBuilderAPI_MakeEdge(outL, gp_Pnt(inL.X(), 0.0, inL.Z()));     // span at z=L
    TopoDS_Edge eBottom = BRepBuilderAPI_MakeEdge(gp_Pnt(in0.X(), 0.0, in0.Z()), out0);  // span at z=0

    // Reverse inner edge orientation so it runs L->0
    TopoDS_Edge eInRev = TopoDS::Edge(eIn.Reversed());

    BRepBuilderAPI_MakeWire wBuilder;
    wBuilder.Add(eOut);
    wBuilder.Add(eTop);
    wBuilder.Add(eInRev);
    wBuilder.Add(eBottom);

    if (!wBuilder.IsDone()) {
        throw std::runtime_error("PipeTransition: wire build failed");
    }

    TopoDS_Wire wire = wBuilder.Wire();

    // Make face from wire
    BRepBuilderAPI_MakeFace fBuilder(wire, /*OnlyPlane*/ false);
    if (!fBuilder.IsDone()) {
        throw std::runtime_error("PipeTransition: face build failed");
    }
    TopoDS_Face face = fBuilder.Face();

    // Revolve around Z
    gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeRevol revol(face, axis, 2.0 * M_PI, /*copy*/ false);
    if (!revol.IsDone()) {
        throw std::runtime_error("PipeTransition: revolve failed");
    }

    TopoDS_Shape solid = revol.Shape();
    return WrapOcctShape(solid);
}
}  // namespace mech
}  // namespace ccad
