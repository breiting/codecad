#pragma once

#include <glm/glm.hpp>

#include "geometry/BaseGeometry.hpp"

class PointCloud : public BaseGeometry {
   public:
    PointCloud();
    ~PointCloud();

    void Upload() override;
    void Render() const override;

   private:
    void deleteBuffers();

   private:
    unsigned int m_Vao, m_Vbo;
};
