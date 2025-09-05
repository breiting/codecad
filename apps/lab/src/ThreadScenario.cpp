#include "ThreadScenario.hpp"

#include <ccad/base/Shape.hpp>
#include <ccad/feature/Chamfer.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/geom/HexPrism.hpp>
#include <ccad/mech/Threads.hpp>
#include <ccad/ops/Boolean.hpp>
#include <ccad/ops/Transform.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

#include "ccad/mech/Rod.hpp"

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::mech;
using namespace ccad::feature;
using namespace pure;

Shape BuildNut(const ThreadSpec& spec) {
    const double height = 10.0;

    // Create the internal cutter
    double boreHoleDiameter;
    auto cutter = mech::ThreadOps::ThreadInternalCutter(spec, height, boreHoleDiameter);

    auto hex = HexPrism(18, height);
    auto hole = Cylinder(boreHoleDiameter, height);

    auto boreHole = ops::Difference(hex, hole);

    auto nut = ops::Difference(boreHole, cutter);

    // apply chamfer
    ChamferRadialSpec radSpec;
    radSpec.type = ChamferRadialType::Internal;
    auto chamfer = ChamferCutterRadial(boreHoleDiameter, radSpec);
    nut = ops::Difference(nut, chamfer);

    chamfer = ops::RotateX(chamfer, 180.0);
    chamfer = ops::Translate(chamfer, 0, 0, height);
    return ops::Difference(nut, chamfer);
}

Shape BuildBolt(double boltLength, double threadLength, const ThreadSpec& spec) {
    RodSpec rodSpec;
    rodSpec.chamferBottom = true;
    rodSpec.chamferTop = true;
    return ThreadedRod(boltLength, threadLength, rodSpec, spec);
}

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    const double boltLength = 30.0;
    const double threadLength = 20.0;

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
    auto bolt = BuildBolt(boltLength, threadLength, spec);
    scene->AddPart("Bolt", ShapeToMesh(bolt), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(bolt);

    // Nut
    auto nut = BuildNut(spec);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(20.f, 0.f, 0.0f));
    scene->AddPart("Top", ShapeToMesh(nut), T, Hex("#ffd2d2"));
    m_Shapes.push_back(nut);
}
