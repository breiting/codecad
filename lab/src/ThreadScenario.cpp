#include "ThreadScenario.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <geometry/Shape.hpp>
#include <geometry/Triangulate.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>
#include <mech/ThreadChamfer.hpp>
#include <mech/ThreadOps.hpp>
#include <mech/ThreadSpec.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

using namespace mech;
using namespace pure;
using namespace geometry;

TopoDS_Shape Fuse(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Fuse op(a, b);
    op.SetRunParallel(true);
    op.SetFuzzyValue(0.001);
    op.Build();
    if (!op.IsDone()) {
        op.DumpErrors(std::cerr);
        throw std::runtime_error("Fuse failed");
    }
    return op.Shape();
}

TopoDS_Shape Cut(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepAlgoAPI_Cut op(a, b);
    op.SetRunParallel(true);
    op.SetFuzzyValue(0.001);
    if (!op.IsDone()) throw std::runtime_error("Cut failed");
    return op.Shape();
}

geometry::ShapePtr BuildCanBottom(double canHeight, double canDiameter, double canWallThickness,
                                  const ThreadSpec& spec) {
    const double threadLength = 15.0;

    const double R_outer = 0.5 * canDiameter;
    const double R_inner = R_outer - canWallThickness - spec.clearance;

    TopoDS_Shape canOuter = BRepPrimAPI_MakeCylinder(R_outer, canHeight).Shape();
    TopoDS_Shape canBore = BRepPrimAPI_MakeCylinder(R_inner, canHeight).Shape();

    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(0, 0, -canWallThickness));
    canBore = BRepBuilderAPI_Transform(canBore, tr, true).Shape();

    TopoDS_Shape canHollow = Cut(canOuter, canBore);

    // Create the internal cutter
    auto cutter = mech::ThreadOps::ThreadInternalCutter(spec, threadLength);

    TopoDS_Shape canWithThread = BRepAlgoAPI_Cut(canHollow, cutter->Get()).Shape();

    // auto chamfered = mech::ChamferThreadEndsInternal(canWithThread, R_inner, 5.0, 45.0, 0, true, true);

    return std::make_shared<Shape>(canWithThread);
    // return std::make_shared<Shape>(chamfered);
}

geometry::ShapePtr BuildCanTop(double lidHandleHeight, double threadLength, double canDiameter,
                               const ThreadSpec& spec) {
    auto lid = mech::ThreadOps::ThreadExternalRod(spec, threadLength, threadLength);

    auto handle = BRepPrimAPI_MakeCylinder(0.5 * canDiameter, lidHandleHeight).Shape();
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(0, 0, -lidHandleHeight));
    handle = BRepBuilderAPI_Transform(handle, tr, true).Shape();

    TopoDS_Shape fused = Fuse(lid->Get(), handle);

    const double holeDiameter = canDiameter - 15.0;
    auto hole = BRepPrimAPI_MakeCylinder(0.5 * holeDiameter, threadLength).Shape();

    TopoDS_Shape lidFinal = Cut(fused, hole);

    return std::make_shared<Shape>(lidFinal);
}

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    const double lidThreadLength = 10.0;  // Thread length for lid
    const double lidHandleHeight = 5.0;   // Handle height
    const double canHeight = 25;
    const double canDiameter = 60.0;  // Aussendurchmesser Dose
    const double canWallThickness = 3;

    ThreadSpec spec;
    spec.fitDiameter = canDiameter - 2 * canWallThickness;  // 40mm
    spec.pitch = 8.0;                                       // coarse, 1 turn per 4mm
    spec.depth = 5;                                         // chunky ridges for print strength
    spec.flankAngleDeg = 60.0;
    spec.clearance = 0.1;  // print fit
    spec.handedness = mech::Handedness::Right;
    spec.tip = mech::TipStyle::Cut;
    spec.tipCutRatio = 0.4;
    spec.segmentsPerTurn = 96;

    // Bottom
    auto canBottom = BuildCanBottom(canHeight, canDiameter, canWallThickness, spec);
    scene->AddPart("Bottom", ShapeToMesh(canBottom->Get()), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(canBottom);

    // Top
    auto canTop = BuildCanTop(lidHandleHeight, lidThreadLength, canDiameter, spec);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(80.f, 0.f, 0.0f));
    scene->AddPart("Top", ShapeToMesh(canTop->Get()), T, Hex("#ffd2d2"));
    m_Shapes.push_back(canTop);
}
#if 0
    double majorD = 24;
    double length = 30;

    auto rod1 = MakeCylinder(majorD, length);
    auto rod1_chamf =
        mech::ChamferThreadEndsExternal(rod1->Get(), majorD / 2, 1.5, 30.0, length, true /*start*/, true /*end*/);

    double rod2Dmaj = 20;
    double rod2Dmin = 15;
    double rod2H = 30;
    auto c1 = MakeCylinder(rod2Dmaj, rod2H);
    auto c2 = MakeCylinder(rod2Dmin, rod2H);
    auto rod2 = MakeDifference(c1, c2);
    auto rod2_chamf =
        mech::ChamferThreadEndsInternal(rod2->Get(), rod2Dmin / 2, 1.5, 30.0, rod2H, true /*start*/, true /*end*/);

    scene->AddPart("Thread", ShapeToMesh(rod1_chamf), glm::mat4{1.0f}, Hex("#d2d2d2"));

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(40.f, 0.f, 0.f));
    scene->AddPart("Thread", ShapeToMesh(rod2_chamf), T, Hex("#d2d2d2"));
#endif
