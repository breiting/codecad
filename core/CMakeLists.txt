include(FetchContent)

# sol2 (header-only) - Lua integration
FetchContent_Declare(
  sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)

# Find Lua (prefer 5.4)
find_package(Lua 5.4 QUIET)
if(NOT Lua_FOUND)
  find_package(Lua 5.3 QUIET)
endif()
if(NOT Lua_FOUND)
  message(FATAL_ERROR "Lua 5.3+ not found. Install lua (e.g., apt install liblua5.4-dev) or set Lua_DIR.")
endif()
message(STATUS "Lua include: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libs: ${LUA_LIBRARIES}")

# GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1   # oder aktuelles Release
)
FetchContent_MakeAvailable(glm)


# JSON
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Find OpenCascade
find_package(OpenCASCADE REQUIRED)
message(STATUS "Found OpenCASCADE: ${OpenCASCADE_VERSION}")

add_library(core STATIC
   src/Boolean.cpp
   src/CoreEngine.cpp
   src/Draft.cpp
   src/Export.cpp
   src/Extrude.cpp
   src/Features.cpp
   src/Gear.cpp
   src/Measure.cpp
   src/Primitives.cpp
   src/Project.cpp
   src/Sketch.cpp
   src/Transform.cpp
   src/Triangulate.cpp
   src/lua/BindAssembly.cpp
   src/lua/BindBooleans.cpp
   src/lua/BindConstruct.cpp
   src/lua/BindDraft.cpp
   src/lua/BindFeatures.cpp
   src/lua/BindGears.cpp
   src/lua/BindIO.cpp
   src/lua/BindMeasure.cpp
   src/lua/BindPrimitives.cpp
   src/lua/BindSketch.cpp
   src/lua/BindTransforms.cpp
   src/lua/BindingUtils.cpp
)

# --- now configure the target ---
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/core
    ${LUA_INCLUDE_DIR}
    ${sol2_SOURCE_DIR}/include
)

# On some systems, OpenCascade doesn't propagate include dirs well
if(DEFINED OpenCASCADE_INCLUDE_DIR)
  target_include_directories(core PRIVATE ${OpenCASCADE_INCLUDE_DIR})
endif()

target_link_libraries(core PRIVATE
    project_settings
    ${OpenCASCADE_LIBRARIES}
    glm::glm
    nlohmann_json::nlohmann_json
    ${LUA_LIBRARIES}
)
