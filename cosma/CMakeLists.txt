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
    cosma_deps
)

