add_executable(ccad_tests
	test_main.cpp
	test_shapes.cpp
)

target_include_directories(ccad_tests PUBLIC
				${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
)



target_link_libraries(ccad_tests PRIVATE
	GTest::gtest
	GTest::gtest_main
	project_settings
	core
)

include(GoogleTest)
gtest_discover_tests(ccad_tests
		DISCOVERY_TIMEOUT 60
)
