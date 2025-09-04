#include <gtest/gtest.h>

#include <ccad/geom/Primitives.hpp>

using namespace ccad;

TEST(TestShapes, Shape_Create) {
    auto box = Box(10, 10, 10);
    auto bbox = box.BBox();
    EXPECT_NEAR(bbox.Size().x, 10, 1e-6);

    EXPECT_ANY_THROW(Cylinder(0, 10));

    auto cyl = Cylinder(5, 10);
    EXPECT_STREQ(cyl.TypeName().c_str(), "OcctShape");

    // auto kernel = std::make_shared<backend::occt::OcctKernel>();
    // geometry::Modeler mdl{kernel};
    //
    // auto box = mdl.Box(10.0, 20.0, 30.0);
    // ASSERT_NE(box, nullptr) << "Box() must return a valid shape";
    //
    // auto bbRes = kernel->bbox(box, /*triangulated=*/false);
    // ASSERT_TRUE(bbRes.has_value()) << "bbox failed: " << bbRes.message();
}
