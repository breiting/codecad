add_library(core STATIC
   src/Boolean.cpp
   src/Draft.cpp
   src/Export.cpp
   src/Extrude.cpp
   src/Features.cpp
   src/Gear.cpp
   src/LuaEngine.cpp
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
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(core PRIVATE
    project_settings
    ${OpenCASCADE_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${LUA_LIBRARIES}
	PUBLIC glm::glm
)
