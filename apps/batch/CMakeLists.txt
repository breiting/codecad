add_executable(batch
	src/main.cpp
)

target_link_libraries(batch PRIVATE
	project_settings
	lua
)

