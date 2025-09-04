#include <gtest/gtest.h>

#include <ccad/geom/Operations.hpp>
#include <ccad/geom/Primitives.hpp>

#include "ccad/geom/Math.hpp"

using namespace ccad;

TEST(TestOps, TestUnion) {
    auto b1 = Box(10, 10, 10);
    auto b2 = Box(10, 10, 10);

    b2 = op::Translate(b2, 10, 0, 0);

    auto fused = op::Union(b1, b2);
    auto bbox = fused.BBox();
    EXPECT_NEAR(bbox.Size().x, 20, 1e-6);
}

TEST(TestOps, TestDifference) {
    auto b1 = Box(10, 10, 10);
    auto b2 = Box(5, 10, 10);

    auto cut = op::Difference(b1, b2);

    auto bbox = cut.BBox();
    EXPECT_NEAR(bbox.Size().x, 5, 1e-6);
    EXPECT_NEAR(bbox.Size().y, 10, 1e-6);
    EXPECT_NEAR(bbox.Size().z, 10, 1e-6);
    EXPECT_NEAR(bbox.min.x, 5, 1e-6);
}
