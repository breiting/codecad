#pragma once
#include <TopoDS_Shape.hxx>
#include <glm/glm.hpp>
#include <memory>
#include <pure/PureScene.hpp>
#include <string>

#include "geometry/Shape.hpp"

/**
 * @brief Interface for small, self-contained modeling demos.
 *
 * A Scenario generates a PureScene based on a specific TopoDS_Shape
 */
class Scenario {
   public:
    virtual ~Scenario() = default;

    /// Human-readable name for UI / window title suffix
    virtual std::string Name() const = 0;
    virtual void Build(std::shared_ptr<pure::PureScene> scene) = 0;

    void SaveSTL(const std::string& fileName);

   protected:
    std::shared_ptr<pure::PureMesh> ShapeToMesh(const TopoDS_Shape& shape);
    glm::vec3 Hex(const std::string& hex);

   protected:
    std::vector<geometry::ShapePtr> m_Shapes;
};

using ScenarioPtr = std::shared_ptr<Scenario>;
