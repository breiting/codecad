# -------------------------
# Pure library
# -------------------------
add_library(pure STATIC
	src/PureAxis.cpp
	src/PureController.cpp
	src/PureGui.cpp
	src/PureMath.cpp
	src/PureMeasurement.cpp
	src/PureMesh.cpp
	src/PureMeshFactory.cpp
	src/PurePerspectiveCamera.cpp
	src/PurePicker.cpp
	src/PureRenderArea.cpp
	src/PureRenderer.cpp
	src/PureScene.cpp
	src/PureShader.cpp

	external/glad/src/gl.cpp
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# -------------------------
# Shaders (embed to headers)
# -------------------------
set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(GEN_INC_DIR    "${CMAKE_BINARY_DIR}/generated")

embed_text_to_header("${SHADER_SRC_DIR}/phong.vert"  "${GEN_INC_DIR}/assets/phong_vert.h"  phong_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/phong.frag"  "${GEN_INC_DIR}/assets/phong_frag.h"  phong_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/unlit.vert"  "${GEN_INC_DIR}/assets/unlit_vert.h"  unlit_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/unlit.frag"  "${GEN_INC_DIR}/assets/unlit_frag.h"  unlit_frag_glsl)


# -------------------------
# Includes
# -------------------------
target_include_directories(pure
				PUBLIC
								${CMAKE_CURRENT_SOURCE_DIR}/inc
								${GEN_INC_DIR}
				PRIVATE
								external/glad/include
								${imgui_SOURCE_DIR}
								${imgui_SOURCE_DIR}/backends
)

# -------------------------
# Linking
# -------------------------
target_link_libraries(pure
				PUBLIC
								project_settings
								glfw
		glm
)
