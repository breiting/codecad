add_executable(test_lua
	main.cpp
	TestBasic.cpp
)

target_include_directories(test_lua PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../inc
)

target_link_libraries(test_lua PRIVATE
	GTest::gtest
	project_settings
	lua
	kernel
)

add_test(
	NAME test_lua
	COMMAND $<TARGET_FILE:test_lua>
)
