add_library(lua STATIC
			src/LuaEngine.cpp
			src/BindIO.cpp
			src/BindPrimitives.cpp
													# src/lua/BindBooleans.cpp
													# src/lua/BindConstruct.cpp
													# src/lua/BindDraft.cpp
													# src/lua/BindFeatures.cpp
													# src/lua/BindPipes.cpp
													# src/lua/BindMeasure.cpp
													# src/lua/BindSketch.cpp
													# src/lua/BindThreads.cpp
													# src/lua/BindTransforms.cpp
													# src/lua/BindingUtils.cpp
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

