add_executable(codecad
	src/App.cpp
    src/main.cpp
)

target_include_directories(codecad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${LUA_INCLUDE_DIR}
    ${sol2_SOURCE_DIR}/include
)

target_link_libraries(codecad PRIVATE 
	project_settings
	CLI11::CLI11
	sol2
)

if(ENABLE_COSMA)
    target_link_libraries(codecad PRIVATE cosma)
	target_compile_definitions(codecad PRIVATE ENABLE_COSMA)
endif()
