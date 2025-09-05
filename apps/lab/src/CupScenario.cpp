#include "CupScenario.hpp"

#include <ccad/base/Shape.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/mech/Threads.hpp>
#include <ccad/ops/Boolean.hpp>
#include <ccad/ops/Transform.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::mech;
using namespace pure;

Shape BuildCanBottom(double canHeight, double canDiameter, double canWallThickness, const ThreadSpec& spec) {
    const double threadLength = 16.0;

    // Create the internal cutter
    double boreHoleDiameter;
    auto cutter = mech::ThreadOps::ThreadInternalCutter(spec, threadLength, boreHoleDiameter);

    auto canOuter = Cylinder(canDiameter, canHeight);
    auto canBore = Cylinder(boreHoleDiameter, canHeight);

    canBore = ops::Translate(canBore, 0, 0, -canWallThickness);

    auto canHollow = ops::Difference(canOuter, canBore);

    return ops::Difference(canHollow, cutter);
}

Shape BuildCanTop(double lidHandleHeight, double threadLength, double canDiameter, double canWallThickness,
                  const ThreadSpec& spec) {
    auto lid = mech::ThreadOps::ThreadExternalRod(spec, threadLength, threadLength);

    auto handle = Cylinder(canDiameter, lidHandleHeight);
    handle = ops::Translate(handle, 0, 0, -lidHandleHeight);

    auto fused = ops::Union(lid, handle);

    const double holeDiameter = spec.fitDiameter - canWallThickness;
    auto hole = Cylinder(holeDiameter, threadLength);

    return ops::Difference(fused, hole);
}

void CupScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    const double lidThreadLength = 10.0;  // Thread length for lid
    const double lidHandleHeight = 5.0;   // Handle height
    const double canHeight = 25;
    const double canDiameter = 62.0;  // Aussendurchmesser Dose
    const double canWallThickness = 4;

    ThreadSpec spec;
    spec.fitDiameter = canDiameter - 2 * canWallThickness;  // 40mm
    spec.pitch = 8.0;                                       // coarse, 1 turn per pitch [mm]
    spec.depth = 3;                                         // chunky ridges for print strength
    spec.flankAngleDeg = 60.0;
    spec.clearance = 0.2;  // print fit only for external thread!
    spec.handedness = mech::Handedness::Right;
    spec.tip = mech::TipStyle::Cut;
    spec.tipCutRatio = 0.4;
    spec.segmentsPerTurn = 96;

    // Bottom
    auto canBottom = BuildCanBottom(canHeight, canDiameter, canWallThickness, spec);
    scene->AddPart("Bottom", ShapeToMesh(canBottom), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(canBottom);

    // Top
    auto canTop = BuildCanTop(lidHandleHeight, lidThreadLength, canDiameter, canWallThickness, spec);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(80.f, 0.f, 0.0f));
    scene->AddPart("Top", ShapeToMesh(canTop), T, Hex("#ffd2d2"));
    m_Shapes.push_back(canTop);
}
