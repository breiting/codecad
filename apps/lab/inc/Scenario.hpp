#pragma once
#include <ccad/base/Shape.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <pure/PureScene.hpp>
#include <string>

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
    std::shared_ptr<pure::PureMesh> ShapeToMesh(const ccad::Shape& shape);
    glm::vec3 Hex(const std::string& hex);

   protected:
    std::vector<ccad::Shape> m_Shapes;
};

using ScenarioPtr = std::shared_ptr<Scenario>;
