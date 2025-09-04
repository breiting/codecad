#include <gtest/gtest.h>

#include <ccad/geom/Box.hpp>

#include "ccad/geom/Triangulation.hpp"

using namespace ccad;
using namespace ccad::geom;

TEST(TestTriMesh, CreateMesh) {
    auto box = Box(1, 1, 1);

    TriangulationParams params;
    auto mesh = Triangulate(box, params);

    std::cout << mesh;
}
