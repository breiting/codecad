cmake_minimum_required(VERSION 3.16)

project(codecad LANGUAGES CXX)

# Produce json file for neovim clangd completion
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Working on C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COSMA "Build with Cosma 3D viewer support" ON)

# Load compiler settings target
include(cmake/CompilerSettings.cmake)

# Load embed text functionality
include(cmake/EmbedText.cmake)

include(FetchContent)

# -------- Dependencies (fetch once, available everywhere) --------
# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# sol2
FetchContent_Declare(
  sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)

# nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# CLI11
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.4.2   # stable version
)
FetchContent_MakeAvailable(CLI11)


# LUA dependency
find_package(Lua 5.4 QUIET)
if(NOT Lua_FOUND)
  find_package(Lua 5.3 QUIET)
endif()
if(NOT Lua_FOUND)
  message(FATAL_ERROR "Lua 5.3+ not found. Install lua (e.g., apt install liblua5.4-dev) or set Lua_DIR.")
endif()
message(STATUS "Lua include: ${LUA_INCLUDE_DIR}")
message(STATUS "Lua libs: ${LUA_LIBRARIES}")

# ---------------------------------------------------------------

add_subdirectory(core)

if(ENABLE_COSMA)
	include(cmake/ConfigCosma.cmake)
	add_subdirectory(cosma)
endif()

add_subdirectory(app)

# ---------------------------------------------------------------
# INSTALL
# ---------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS ccad
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# --- Lua library + types + docs ---
set(CODECAD_SHARE_DIR ${CMAKE_INSTALL_DATADIR}/codecad)

install(DIRECTORY lib/
  DESTINATION ${CODECAD_SHARE_DIR}/lib
  FILES_MATCHING PATTERN "*.lua"
)

install(DIRECTORY types/
  DESTINATION ${CODECAD_SHARE_DIR}/types
  FILES_MATCHING PATTERN "*.d.lua"
)

# Install fonts
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts
    DESTINATION share/codecad
    FILES_MATCHING PATTERN "*.ttf"
)
