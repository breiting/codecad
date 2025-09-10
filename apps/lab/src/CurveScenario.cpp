#include "CurveScenario.hpp"

#include <ccad/base/Shape.hpp>
#include <ccad/construct/Revolve.hpp>
#include <ccad/geom/Curves.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/ops/Boolean.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

#include "ccad/geom/CurvedPlate.hpp"
#include "glm/ext/matrix_transform.hpp"

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::ops;
using namespace pure;

void CurveScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    double diaBottom = 20;
    std::vector<Vec2> points{{diaBottom * 0.5, 0}, {25, 10}, {10, 50}, {10, 35}, {10, 75}};

    LatheSpec spec;
    spec.points = points;
    spec.angleDeg = 360;
    spec.thickness = 2;
    auto shape = Lathe(spec);
    auto bottom = Cylinder(diaBottom, 5);
    shape = Union({shape, bottom});

    CurvedPlateSpec plateSpec;
    plateSpec.kU = 0.002;
    plateSpec.kV = 0.05;
    auto plate = CurvedPlate(plateSpec);

    scene->AddPart("Curve", ShapeToMesh(shape), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(shape);

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(20.f, 0.f, 0.0f));
    scene->AddPart("Plate", ShapeToMesh(plate), T, Hex("#ffffd2"));
    m_Shapes.push_back(shape);
}
