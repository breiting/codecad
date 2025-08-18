# GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1   # oder aktuelles Release
)
FetchContent_MakeAvailable(glm)



add_library(cosma STATIC
    src/CosmaViewer.cpp
)

target_include_directories(cosma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
)

target_link_libraries(cosma PUBLIC
    project_settings
    core
	glm::glm
    cosma_deps
)

