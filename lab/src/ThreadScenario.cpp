#include "ThreadScenario.hpp"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <geometry/Shape.hpp>
#include <geometry/Triangulate.hpp>
#include <glm/glm.hpp>
#include <mech/CoarseThread.hpp>
#include <mech/ThreadChamfer.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

#include "geometry/Boolean.hpp"
#include "geometry/Primitives.hpp"
#include "glm/ext/matrix_transform.hpp"

using namespace pure;
using namespace geometry;

void ThreadScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    // --- Parameters for a chunky 1-turn bolt + matching nut ---
    const double length = 50.0;     // bolt thread length
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
}
