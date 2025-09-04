#include <gtest/gtest.h>

#include <ccad/geom/Primitives.hpp>

using namespace ccad;

TEST(TestShapes, Shape_Create) {
    auto box = Box(10, 10, 10);
    auto bbox = box.BBox();
    EXPECT_NEAR(bbox.size().x, 10, 1e-6);

    EXPECT_ANY_THROW(Cylinder(0, 10));

    // auto kernel = std::make_shared<backend::occt::OcctKernel>();
    // geometry::Modeler mdl{kernel};
    //
    // auto box = mdl.Box(10.0, 20.0, 30.0);
    // ASSERT_NE(box, nullptr) << "Box() must return a valid shape";
    //
    // auto bbRes = kernel->bbox(box, /*triangulated=*/false);
    // ASSERT_TRUE(bbRes.has_value()) << "bbox failed: " << bbRes.message();
    //
    // auto bb = bbRes.value();
    // EXPECT_TRUE(bb.valid);
    // EXPECT_NEAR(bb.max.x - bb.min.x, 10.0, 1e-6);
    // EXPECT_NEAR(bb.max.y - bb.min.y, 20.0, 1e-6);
    // EXPECT_NEAR(bb.max.z - bb.min.z, 30.0, 1e-6);
}
