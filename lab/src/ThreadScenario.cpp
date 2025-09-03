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

geometry::ShapePtr BuildNut(const ThreadSpec& spec) {
    const double height = 10.0;

    // Create the internal cutter
    double boreHoleDiameter;
    auto cutter = mech::ThreadOps::ThreadInternalCutter(spec, height, boreHoleDiameter);

    TopoDS_Shape hex = BRepPrimAPI_MakeCylinder(8.5, height).Shape();
    TopoDS_Shape hole = BRepPrimAPI_MakeCylinder(boreHoleDiameter * 0.5, height).Shape();

    TopoDS_Shape boreHole = BRepAlgoAPI_Cut(hex, hole);

    TopoDS_Shape nut = BRepAlgoAPI_Cut(boreHole, cutter->Get()).Shape();

    auto chamfered = mech::ChamferThreadEndsInternal(nut, spec.fitDiameter * 0.5, 1.0, 45.0, height, true, true);

    // return std::make_shared<Shape>(cutter->Get());
    // return std::make_shared<Shape>(nut);
    return std::make_shared<Shape>(chamfered);
}

geometry::ShapePtr BuildBolt(double length, const ThreadSpec& spec) {
    auto bolt = mech::ThreadOps::ThreadExternalRod(spec, length, length);

    return bolt;
}

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    const double boltLength = 30.0;

    ThreadSpec spec;
    spec.fitDiameter = 8.0;         // Nenndurchmesser (Major Diameter ISO M8)
    spec.pitch = 1.25;              // Steigung (coarse ISO M8)
    spec.depth = 0.6 * spec.pitch;  // vereinfachte effektive Tiefe ~ 0.75mm
    spec.flankAngleDeg = 60.0;      // ISO Standard
    spec.clearance = 0.0;           // etwas mehr Luft für 3D-Druck
    spec.handedness = mech::Handedness::Right;
    spec.tip = mech::TipStyle::Cut;
    spec.tipCutRatio = 0.4;
    spec.segmentsPerTurn = 64;

    // Bolt
    auto bolt = BuildBolt(boltLength, spec);
    scene->AddPart("Bolt", ShapeToMesh(bolt->Get()), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(bolt);

    // Nut
    auto nut = BuildNut(spec);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(20.f, 0.f, 0.0f));
    scene->AddPart("Top", ShapeToMesh(nut->Get()), T, Hex("#ffd2d2"));
    m_Shapes.push_back(nut);
}
