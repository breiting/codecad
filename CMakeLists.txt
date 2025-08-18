cmake_minimum_required(VERSION 3.16)

project(codecad LANGUAGES CXX)

# Produce json file for neovim clangd completion
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Working on C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ENABLE_COSMA "Build with Cosma 3D viewer support" ON)

# Load compiler settings target
include(cmake/CompilerSettings.cmake)

# Load embed text functionality
include(cmake/EmbedText.cmake)

add_subdirectory(core)

if(ENABLE_COSMA)
	include(cmake/ConfigCosma.cmake)
	add_subdirectory(cosma)
endif()

add_subdirectory(app)





