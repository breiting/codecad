add_executable(clab
	src/App.cpp
	src/CupScenario.cpp
	src/CurveScenario.cpp
	src/FilletScenario.cpp
	src/Scenario.cpp
	src/PoissonScenario.cpp
	src/ThreadScenario.cpp
	src/main.cpp
)

target_include_directories(clab PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends
)

target_link_libraries(clab PRIVATE
	project_settings
	lua
	pure
)

target_compile_definitions(clab PRIVATE ENABLE_PURE)
