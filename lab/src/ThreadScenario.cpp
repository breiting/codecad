#include "ThreadScenario.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <geometry/Shape.hpp>
#include <geometry/Triangulate.hpp>
#include <glm/glm.hpp>
#include <mech/CoarseThread.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

#include "geometry/Primitives.hpp"

using namespace pure;

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    // --- Parameters for a chunky 1-turn bolt + matching nut ---
    const double length = 30.0;     // bolt thread length
    const double turns = 1.0;       // one revolution
    const double depth = 2.5;       // thread radial height
    const double majorD = 24.0;     // outer diameter of thread crest
    const double clearance = 0.25;  // add a bit for 3D print fit

    mech::CoarseThreadParams tp;
    tp.length = length;
    tp.turns = (int)turns;
    tp.depth = depth;
    tp.leftHand = false;
    tp.flankAngleDeg = 60.0;
    tp.clearance = 0.0;  // external thread itself: zero; nut will add clearance

    auto box = geometry::MakeBox(10, 10, 10);

    scene->AddPart("Box", ShapeToMesh(box->Get()), glm::mat4{1.0f}, Hex("#d2d2d2"));
}
