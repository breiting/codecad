#include "PoissonScenario.hpp"

#include <ccad/base/Shape.hpp>
#include <ccad/construct/Revolve.hpp>
#include <ccad/geom/Box.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/ops/Boolean.hpp>
#include <iostream>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

#include "ccad/base/PoissonDisk.hpp"
#include "ccad/geom/Poisson.hpp"

using namespace std;

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::ops;
using namespace pure;

void PoissonScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    auto width = 130;      // cm
    auto height = 170;     // cm
    auto thickness = 0.5;  // cm

    PoissonDiskSpec spec;
    spec.width = width;
    spec.height = height;
    spec.margin = 4;
    spec.targetPoints = 200;
    spec.rMin = 0.5;
    spec.rMax = 3.5;
    spec.seed = 42;
    spec.densityFalloff = 0.5;

    PoissonDiskGenerator gen(spec);
    PoissonResult res = gen.Run();
    cout << "Generated " << res.points.size() << " points" << endl;

    auto plate = Poisson(spec, thickness, res);

    scene->AddPart("Poisson", ShapeToMesh(plate), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(plate);
}
