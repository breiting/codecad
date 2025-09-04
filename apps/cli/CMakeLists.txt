add_executable(ccad
	src/App.cpp
	src/Controller.cpp
	src/ProjectPanel.cpp
	src/Utils.cpp
				src/main.cpp
)

# -------------------------
# Assets (embed to headers)
# -------------------------
set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(GEN_INC_DIR    "${CMAKE_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${GEN_INC_DIR}/assets")

embed_text_to_header("${ASSETS_SRC_DIR}/readme_template.md"     "${GEN_INC_DIR}/assets/readme_template.h"   readme_template)
embed_text_to_header("${ASSETS_SRC_DIR}/part_template.lua"      "${GEN_INC_DIR}/assets/part_template.h"     part_template)


target_include_directories(ccad PUBLIC
				${CMAKE_CURRENT_SOURCE_DIR}/../../core/inc
				${CMAKE_CURRENT_SOURCE_DIR}/../../libs/pure/inc
				${CMAKE_CURRENT_SOURCE_DIR}/inc
				${LUA_INCLUDE_DIR}
				${sol2_SOURCE_DIR}/include
				${CMAKE_BINARY_DIR}/generated
				${imgui_SOURCE_DIR}
				${imgui_SOURCE_DIR}/backends
)

target_link_libraries(ccad PRIVATE
	project_settings
	CLI11::CLI11
	sol2
	glfw
	core
	pure
	nlohmann_json::nlohmann_json
)

if(ENABLE_PURE)
	target_link_libraries(ccad PRIVATE pure)
	target_compile_definitions(ccad PRIVATE ENABLE_PURE)
endif()
