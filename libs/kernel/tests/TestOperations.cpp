#include <gtest/gtest.h>

#include <ccad/geom/Box.hpp>
#include <ccad/ops/Boolean.hpp>
#include <ccad/ops/Transform.hpp>
#include <ccad/sketch/Rectangle.hpp>

#include "ccad/construct/Extrude.hpp"

using namespace ccad;
using namespace ccad::geom;
using namespace ccad::sketch;

TEST(TestOps, TestUnion) {
    auto b1 = Box(10, 10, 10);
    auto b2 = Box(10, 10, 10);

    b2 = ops::Translate(b2, 10, 0, 0);

    auto fused = ops::Union(b1, b2);
    auto bbox = fused.BBox();
    EXPECT_NEAR(bbox.Size().x, 20, 1e-6);
}

TEST(TestOps, TestDifference) {
    auto b1 = Box(10, 10, 10);
    auto b2 = Box(5, 10, 10);

    auto cut = ops::Difference(b1, b2);

    auto bbox = cut.BBox();
    EXPECT_NEAR(bbox.Size().x, 5, 1e-6);
    EXPECT_NEAR(bbox.Size().y, 10, 1e-6);
    EXPECT_NEAR(bbox.Size().z, 10, 1e-6);
    EXPECT_NEAR(bbox.min.x, 5, 1e-6);
}

TEST(TestOps, TestExtrude) {
    auto rect = Rectangle(5, 10);
    auto box = construct::ExtrudeZ(rect, 10);
    auto bbox = box.BBox();
    EXPECT_NEAR(bbox.Size().z, 10, 1e-6);
}
