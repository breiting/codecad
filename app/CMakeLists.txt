# GLM library
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1   # oder aktuelles Release
)
FetchContent_MakeAvailable(glm)


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

add_executable(codecad
    src/main.cpp
)

target_include_directories(codecad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
    ${LUA_INCLUDE_DIR}
    ${sol2_SOURCE_DIR}/include
)

target_link_libraries(codecad PRIVATE 
	project_settings
	glm::glm
)

if(ENABLE_COSMA)
    target_link_libraries(codecad PRIVATE cosma)
	target_compile_definitions(codecad PRIVATE ENABLE_COSMA)
ELSE()
    target_link_libraries(codecad PRIVATE core)
endif()
