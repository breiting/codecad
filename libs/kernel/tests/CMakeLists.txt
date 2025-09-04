add_executable(test_kernel
	main.cpp
	TestShapes.cpp
	TestOperations.cpp
)

target_include_directories(test_kernel PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../inc
)

target_link_libraries(test_kernel PRIVATE
	GTest::gtest
	project_settings
	kernel
)

add_test(
	NAME test_kernel
	COMMAND $<TARGET_FILE:test_kernel>
)
