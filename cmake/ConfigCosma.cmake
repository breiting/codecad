# ConfigCosma.cmake
#
# This file is only included if ENABLE_COSMA=ON.
# It finds all required dependencies for the Cosma viewer.

message(STATUS "Configuring Cosma viewer...")

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    message(STATUS "Found SDL2: ${SDL2_INCLUDE_DIRS}")
endif()

find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
endif()

# optional weitere libs (GLAD, GLEW, ImGui usw.)
# find_package(GLEW REQUIRED)
# target_link_libraries(cosma PRIVATE GLEW::GLEW)

# Exportieren als INTERFACE target, damit cosma es einfach übernehmen kann
add_library(cosma_deps INTERFACE)

target_link_libraries(cosma_deps INTERFACE
    OpenGL::GL
    SDL2::SDL2
)

target_include_directories(cosma_deps INTERFACE
    ${SDL2_INCLUDE_DIRS}
)

