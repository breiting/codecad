#include <gtest/gtest.h>

#include <ccad/geom/Box.hpp>
#include <ccad/geom/Cylinder.hpp>
#include <ccad/mech/Rod.hpp>

using namespace ccad;
using namespace ccad::mech;

TEST(TestMech, Rod) {
    RodSpec spec;
    spec.length = 20.0;
    auto rod = Rod(spec);

    auto bbox = rod.BBox();
    EXPECT_NEAR(bbox.Size().z, 20, 1e-6);
}
