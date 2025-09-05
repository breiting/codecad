#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // Place to init global state/flags if needed
    auto ret = RUN_ALL_TESTS();
    printf("Return code %d\n", ret);
    return ret;
}
