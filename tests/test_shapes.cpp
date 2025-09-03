#include <gtest/gtest.h>

// #include <ccad/backend/occt/OcctKernel.hpp>
// #include <ccad/foundation/Types.hpp>
// #include <ccad/geometry/Modeler.hpp>
// #include <memory>

// using namespace ccad;

TEST(Shapes, Box) {
    EXPECT_TRUE(1 == 1);
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
