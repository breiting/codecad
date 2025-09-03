add_executable(test_ccad
	test_main.cpp
	test_shapes.cpp
)

target_include_directories(test_ccad PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
)

target_link_libraries(test_ccad PRIVATE
	GTest::gtest
	project_settings
	core
)

add_test(
	NAME test_ccad
	COMMAND $<TARGET_FILE:test_ccad>
)
