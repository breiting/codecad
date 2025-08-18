add_executable(codecad
    src/main.cpp
)

target_include_directories(codecad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
)

target_link_libraries(codecad PRIVATE 
project_settings
core)

if(ENABLE_COSMA)
    target_link_libraries(codecad PRIVATE cosma)
    target_compile_definitions(codecad PRIVATE ENABLE_COSMA)
endif()



#
#
#
#
# # sol2 via FetchContent (header-only)
# include(FetchContent)
# FetchContent_Declare(
#   sol2
#   GIT_REPOSITORY https://github.com/ThePhD/sol2.git
#   GIT_TAG v3.3.0
# )
# FetchContent_MakeAvailable(sol2)
#
# # Find Lua (prefer 5.4)
# find_package(Lua 5.4 QUIET)
# if(NOT Lua_FOUND)
#   find_package(Lua 5.3 QUIET)
# endif()
# if(NOT Lua_FOUND)
#   message(FATAL_ERROR "Lua 5.3+ not found. Install lua (e.g., apt install liblua5.4-dev) or set Lua_DIR.")
# endif()
# message(STATUS "Lua include: ${LUA_INCLUDE_DIR}")
# message(STATUS "Lua libs: ${LUA_LIBRARIES}")
#
# # Find OpenCascade
# find_package(OpenCASCADE REQUIRED)
# message(STATUS "Found OpenCASCADE: ${OpenCASCADE_VERSION}")
#
# add_executable(ccad
#   src/main.cpp
#   src/LuaBindings.cpp
#   src/lua/BindingUtils.cpp
#   src/lua/BindPrimitives.cpp
#   src/lua/BindTransforms.cpp
#   src/lua/BindAssembly.cpp
#   src/lua/BindDraft.cpp
#   src/lua/BindBooleans.cpp
#   src/lua/BindMeasure.cpp
#   src/lua/BindFeatures.cpp
#   src/lua/BindSketch.cpp
#   src/lua/BindConstruct.cpp
#   src/lua/BindGears.cpp
#   src/lua/BindIO.cpp
#   src/Primitives.cpp
#   src/Transform.cpp
#   src/Measure.cpp
#   src/Boolean.cpp
#   src/Draft.cpp
#   src/Extrude.cpp
#   src/Gear.cpp
#   src/Sketch.cpp
#   src/Features.cpp
#   src/Export.cpp
#   src/Manifest.cpp
# )
#
# target_include_directories(ccad PRIVATE
#   ${CMAKE_CURRENT_SOURCE_DIR}/inc
#   ${LUA_INCLUDE_DIR}
#   ${sol2_SOURCE_DIR}/include
# )
#
# target_link_libraries(ccad PRIVATE
#   ${OpenCASCADE_LIBRARIES}
#   ${LUA_LIBRARIES}
# )
#
# # On some systems, OpenCascade doesn't propagate include dirs well
# if(DEFINED OpenCASCADE_INCLUDE_DIR)
#   target_include_directories(ccad PRIVATE ${OpenCASCADE_INCLUDE_DIR})
# endif()
#
# # Helpful compile flags
# if (MSVC)
#   target_compile_options(ccad PRIVATE /W4)
# else()
# 	target_compile_options(ccad PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-warning-option)
# endif()
