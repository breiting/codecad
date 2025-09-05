add_library(lua STATIC
			src/LuaEngine.cpp
			src/BindIO.cpp
			src/BindPrimitives.cpp
			src/BindTransforms.cpp
)

target_include_directories(lua PUBLIC
				${CMAKE_CURRENT_SOURCE_DIR}/../kernel/inc
				${CMAKE_CURRENT_SOURCE_DIR}/inc
				${LUA_INCLUDE_DIR}
				${sol2_SOURCE_DIR}/include
)

target_link_libraries(lua PRIVATE
				project_settings
				${LUA_LIBRARIES}
)

add_subdirectory(tests)

