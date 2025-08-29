# -------------------------
# Pure library
# -------------------------
add_library(pure STATIC
  src/PureCamera.cpp
  src/PureController.cpp
  src/PureGui.cpp
  src/PureAxis.cpp
  src/PureMesh.cpp
  src/PureMeshFactory.cpp
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
# Includes
# -------------------------
target_include_directories(pure
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
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
