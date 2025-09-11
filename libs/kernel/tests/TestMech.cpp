#include <gtest/gtest.h>

#include <ccad/geom/Box.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/mech/Rod.hpp>

using namespace ccad;
using namespace ccad::mech;

TEST(TestMech, Rod) {
    RodSpec spec;
    auto rod = Rod(20.0, 5.0, spec);

    auto bbox = rod.BBox();
    EXPECT_NEAR(bbox.Size().z, 5, 1e-6);
}
