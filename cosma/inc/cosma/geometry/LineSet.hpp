#pragma once

#include <glm/glm.hpp>

#include "geometry/BaseGeometry.hpp"

class LineSet : public BaseGeometry {
   public:
    LineSet();
    ~LineSet();

    void Upload() override;
    void Render() const override;

   private:
    void deleteBuffers();

   private:
    unsigned int m_Vao, m_Vbo;
};
