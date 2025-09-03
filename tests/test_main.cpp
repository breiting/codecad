#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // Place to init global state/flags if needed
    return RUN_ALL_TESTS();
}
