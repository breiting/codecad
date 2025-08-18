# GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1   # oder aktuelles Release
)
FetchContent_MakeAvailable(glm)

add_executable(codecad
    src/main.cpp
)

target_include_directories(codecad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
)

target_link_libraries(codecad PRIVATE 
	project_settings
	glm::glm
	core
)

if(ENABLE_COSMA)
    target_link_libraries(codecad PRIVATE cosma)
	target_compile_definitions(codecad PRIVATE ENABLE_COSMA)
endif()
