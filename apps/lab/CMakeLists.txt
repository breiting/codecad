add_executable(clab
	src/App.cpp
	src/Scenario.cpp
	src/CupScenario.cpp
	src/ThreadScenario.cpp
	src/main.cpp
)

target_include_directories(clab PUBLIC
			# ${CMAKE_CURRENT_SOURCE_DIR}/../pure/inc
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(clab PRIVATE
	project_settings
	kernel
	pure
)

target_compile_definitions(clab PRIVATE ENABLE_PURE)
