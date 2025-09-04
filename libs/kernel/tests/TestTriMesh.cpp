#include <gtest/gtest.h>

#include <ccad/geom/Box.hpp>

#include "ccad/geom/Triangulation.hpp"
#include "ccad/io/Export.hpp"

using namespace ccad;
using namespace ccad::geom;

TEST(TestTriMesh, CreateMesh) {
    auto box = Box(1, 1, 1);

    TriangulationParams params;
    auto mesh = Triangulate(box, params);

    io::SaveSTL(box, "box.stl", params);

    std::cout << mesh;
}
