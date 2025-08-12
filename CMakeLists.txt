cmake_minimum_required(VERSION 3.16)

# Produce json file for neovim clangd completion
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

project(cad_mvp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(BUILD_VIEWER "Build Qt viewer (not used in MVP)" OFF)

# sol2 via FetchContent (header-only)
include(FetchContent)
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

# Find OpenCascade
find_package(OpenCASCADE REQUIRED)
message(STATUS "Found OpenCASCADE: ${OpenCASCADE_VERSION}")

add_executable(cad
  src/main.cpp
  src/LuaBindings.cpp
  src/GeoPrimitives.cpp
  src/GeoTransform.cpp
  src/GeoBoolean.cpp
  src/GeoFeatures.cpp
  src/IOExport.cpp
  src/IOManifest.cpp
)

target_include_directories(cad PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${LUA_INCLUDE_DIR}
  ${sol2_SOURCE_DIR}/include
)

target_link_libraries(cad PRIVATE
  ${OpenCASCADE_LIBRARIES}
  ${LUA_LIBRARIES}
)

# On some systems, OpenCascade doesn't propagate include dirs well
if(DEFINED OpenCASCADE_INCLUDE_DIR)
  target_include_directories(cad PRIVATE ${OpenCASCADE_INCLUDE_DIR})
endif()

# Helpful compile flags
if (MSVC)
  target_compile_options(cad PRIVATE /W4)
else()
	target_compile_options(cad PRIVATE -Wall -Wextra -Wpedantic -Wno-unknown-warning-option)
endif()

# Installation
install(TARGETS cad RUNTIME DESTINATION bin)

# Examples
add_custom_target(run_example
  COMMAND cad build ${CMAKE_CURRENT_SOURCE_DIR}/examples/hello_box.lua -o ${CMAKE_CURRENT_BINARY_DIR}/out
  DEPENDS cad
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Running hello_box.lua to generate STL"
)
