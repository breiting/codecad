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
	sol2
)

if(ENABLE_COSMA)
    target_link_libraries(codecad PRIVATE cosma)
	target_compile_definitions(codecad PRIVATE ENABLE_COSMA)
endif()
