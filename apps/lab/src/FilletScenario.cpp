#include "FilletScenario.hpp"

#include <ccad/base/Shape.hpp>
#include <ccad/feature/Chamfer.hpp>
#include <ccad/feature/Fillet.hpp>
#include <ccad/geom/Box.hpp>
#include <ccad/select/EdgeSelector.hpp>
#include <memory>
#include <pure/PureMesh.hpp>
#include <pure/PureMeshFactory.hpp>
#include <pure/PureScene.hpp>

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::feature;
using namespace ccad::select;
using namespace pure;

void FilletScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    auto box = Box(10, 10, 10);

    auto edges = EdgeSelector::FromShape(box)   //
                     .onBoxSide(BoxSide::ZMin)  //
                     .parallelTo(Axis::X)       //
                     .collect();

    box = Fillet(box, edges, 1);

    edges = EdgeSelector::FromShape(box)   //
                .onBoxSide(BoxSide::ZMax)  //
                .parallelTo(Axis::Y)       //
                .collect();

    box = Chamfer(box, edges, 1);

    scene->AddPart("Box", ShapeToMesh(box), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(box);
}
