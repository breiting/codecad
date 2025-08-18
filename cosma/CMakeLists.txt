include(FetchContent)

# -------------------------
# GLM (header-only)
# -------------------------
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# -------------------------
# JSON (header-only)
# -------------------------
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# -------------------------
# GLFW
# -------------------------
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4   # oder aktuelles Release
)
FetchContent_MakeAvailable(glfw)

# -------------------------
# IMGUI sources
# -------------------------
file(GLOB IMGUI
    external/imgui/*.cpp
    external/imgui/backends/*.cpp
)

# -------------------------
# Cosma library
# -------------------------
add_library(cosma STATIC
    src/CosmaViewer.cpp
    src/ArrowNode.cpp
    src/AxesNode.cpp
    src/BaseGeometry.cpp
    src/BoxNode.cpp
    src/CameraOrbit.cpp
    src/CameraPath.cpp
    src/ConeNode.cpp
	src/CosmaController.cpp
    src/CosmaViewer.cpp
    src/CylinderNode.cpp
    src/DirectionalLight.cpp
    src/Engine.cpp
    src/FlatShadedMaterial.cpp
    src/GridNode.cpp
    src/GuiLayer.cpp
    src/ImageNode.cpp
    src/InfinityGridMaterial.cpp
    src/InfinityGridNode.cpp
    src/LandscapeNode.cpp
    src/LineSet.cpp
    src/LineSetMaterial.cpp
    src/Mesh.cpp
    src/MeshNode.cpp
    src/PhysicsWorld.cpp
    src/PointCloud.cpp
    src/Renderable.cpp
    src/Renderer.cpp
    src/RigidBody.cpp
    src/RigidBodyComponent.cpp
    src/Scene.cpp
    src/SceneNode.cpp
    src/Shader.cpp
    src/ShapeNode.cpp
    src/SphereNode.cpp
    src/Stl.cpp
    src/Texture.cpp
    src/TriangleNode.cpp
    src/Window.cpp
    ${IMGUI}
    external/glad/src/gl.cpp
)

# -------------------------
# Shaders (embed to headers)
# -------------------------
set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(GEN_INC_DIR    "${CMAKE_BINARY_DIR}/generated")

file(MAKE_DIRECTORY "${GEN_INC_DIR}/assets")

embed_text_to_header("${SHADER_SRC_DIR}/flat.vert"     "${GEN_INC_DIR}/assets/flat_vert.h"     flat_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/flat.frag"     "${GEN_INC_DIR}/assets/flat_frag.h"     flat_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/grid.vert"     "${GEN_INC_DIR}/assets/grid_vert.h"     grid_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/grid.frag"     "${GEN_INC_DIR}/assets/grid_frag.h"     grid_frag_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/lineset.vert"  "${GEN_INC_DIR}/assets/lineset_vert.h"  lineset_vert_glsl)
embed_text_to_header("${SHADER_SRC_DIR}/lineset.frag"  "${GEN_INC_DIR}/assets/lineset_frag.h"  lineset_frag_glsl)

# -------------------------
# Includes
# -------------------------
target_include_directories(cosma
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/cosma
        ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
        ${GEN_INC_DIR}
    PRIVATE
        external/glad/include
        external/stb
        external/imgui
        external/imgui/backends
)

# -------------------------
# Linking
# -------------------------
target_link_libraries(cosma
    PUBLIC
        project_settings 
        core
        glm::glm
        glfw
        nlohmann_json::nlohmann_json
        cosma_deps
)

