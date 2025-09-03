#include "GearScenario.hpp"

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
#include <mech/Gear.hpp>
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

void GearScenario::Build(std::shared_ptr<PureScene> scene) {
    scene->Clear();

    GearSpec params;
    params.teeth = 16;
    params.module = 2.0;
    params.thickness = 6.0;
    params.bore = 5.0;

    auto gear = SimpleGear::MakeSpur(params);

    scene->AddPart("Gear", ShapeToMesh(gear->Get()), glm::mat4{1.0f}, Hex("#d2ffd2"));
    m_Shapes.push_back(gear);
}
