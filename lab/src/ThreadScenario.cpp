#include "ThreadScenario.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <geometry/Shape.hpp>
#include <geometry/Triangulate.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
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

geometry::ShapePtr BuildJarBox(const ThreadSpec& spec) {
    const double height = 30.0;
    const double wall = 2.5;
    const double boreDiameter = spec.majorDiameter - 2 * wall;  // inner hole

    TopoDS_Shape jarOuter = BRepPrimAPI_MakeCylinder(0.5 * spec.majorDiameter, height).Shape();
    TopoDS_Shape jarBore = BRepPrimAPI_MakeCylinder(0.5 * boreDiameter, height).Shape();

    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(0, 0, -5));
    jarBore = BRepBuilderAPI_Transform(jarBore, tr, true).Shape();

    TopoDS_Shape jarHollow = Cut(jarOuter, jarBore);

    // Internal thread cutter (20 mm deep)
    auto cutter = mech::ThreadOps::ThreadInternalCutter(spec, boreDiameter, /*threadLength*/ 20.0);
    TopoDS_Shape jarWithThread = BRepAlgoAPI_Cut(jarHollow, cutter->Get()).Shape();

    return std::make_shared<Shape>(jarWithThread);
}

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    ThreadSpec spec;
    spec.majorDiameter = 45.0;  // major diameter (outer of male)
    spec.pitch = 8.0;           // coarse, 1 turn per 4mm
    spec.depth = 3;             // chunky ridges for print strength
    spec.flankAngleDeg = 60.0;
    spec.clearance = 0.25;  // print fit
    spec.handedness = mech::Handedness::Right;
    spec.tip = mech::TipStyle::Cut;
    spec.tipCutRatio = 0.4;
    spec.segmentsPerTurn = 96;

    auto part = BuildJarBox(spec);
    scene->AddPart("Box", ShapeToMesh(part->Get()), glm::mat4{1.0f}, Hex("#d2d2d2"));

    m_Shapes.push_back(part);

    const double lidHeight = 15.0;
    const double chamferAngle = 30.0;
    const double chamferHeight = 1.5;
    auto lid = mech::ThreadOps::ThreadExternalRod(spec, /*rodLength*/ lidHeight, /*threadLength*/ lidHeight);
    // auto lid_chamfered =
    //     mech::ChamferThreadEndsExternal(lid->Get(), spec.majorDiameter / 2, chamferHeight, chamferAngle, lidHeight);

    // Griff
    TopoDS_Shape deckel = BRepPrimAPI_MakeCylinder(0.5 * spec.majorDiameter, 5).Shape();
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(0, 0, lidHeight));
    deckel = BRepBuilderAPI_Transform(deckel, tr, true).Shape();

    auto top = Fuse(lid->Get(), deckel);

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(60.f, 0.f, 0.f));
    scene->AddPart("Lid", ShapeToMesh(top), T, Hex("#d2d2d2"));
    m_Shapes.push_back(std::make_shared<Shape>(top));

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
}
