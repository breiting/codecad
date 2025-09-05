cmake_minimum_required(VERSION 3.28)

project(codecad LANGUAGES CXX)

# Version numbering
set (CODECAD_VERSION_MAJOR 0)
set (CODECAD_VERSION_MINOR 1)
set (CODECAD_VERSION_PATCH 0)

if ($ENV{BUILD_NUMBER})
	set (CODECAD_BUILD_NUMBER $ENV{BUILD_NUMBER})
else()
	set (CODECAD_BUILD_NUMBER 11111)
endif()

configure_file (
	${CMAKE_CURRENT_SOURCE_DIR}/libs/kernel/inc/internal/version.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/generated/ccad/version.hpp
	@ONLY
)


# Produce json file for neovim clangd completion
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Working on C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Load compiler settings target
include(cmake/CompilerSettings.cmake)

# Load embed text functionality and create assets folder
include(cmake/EmbedText.cmake)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/assets")

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
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# Dear ImGui
FetchContent_Declare(imgui
		GIT_REPOSITORY https://github.com/ocornut/imgui.git
		GIT_TAG        v1.92.2b-docking
)
FetchContent_MakeAvailable(imgui)

# CLI11
FetchContent_Declare(
		CLI11
		GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
		GIT_TAG v2.4.2   # stable version
)
FetchContent_MakeAvailable(CLI11)

# Google Test
FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

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

enable_testing()

add_subdirectory(libs)
add_subdirectory(apps)

# ---------------------------------------------------------------
# INSTALL
# ---------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS ccad
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# --- Lua library + types + docs ---
set(CODECAD_SHARE_DIR ${CMAKE_INSTALL_DATADIR}/codecad)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs/kernel/inc
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING
	PATTERN "*.hpp"
	PATTERN "*/internal/*" EXCLUDE)

install(DIRECTORY lualib/lib/
		DESTINATION ${CODECAD_SHARE_DIR}/lib
		FILES_MATCHING PATTERN "*.lua"
)

install(DIRECTORY lualib/types/
		DESTINATION ${CODECAD_SHARE_DIR}/types
		FILES_MATCHING PATTERN "*.d.lua"
)

# Install fonts
install(
				DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts
				DESTINATION share/codecad
				FILES_MATCHING PATTERN "*.ttf"
)
