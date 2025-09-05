# cmake/CompilerSettings.cmake
#
# Defines an INTERFACE target `project_warnings`
# that provides a consistent set of compiler warnings
# across all your targets, without affecting third-party libs.

add_library(project_settings INTERFACE)

if (MSVC)
	target_compile_options(project_settings INTERFACE
				/W4                # Level 4 warnings
				/permissive-       # stricter C++ standard conformance
				)
else()
	target_compile_options(project_settings INTERFACE
				-Wall
				-Wextra
				-Wpedantic
				-Wno-unknown-warning-option
				)
endif()

target_compile_definitions(project_settings INTERFACE
	GLM_ENABLE_EXPERIMENTAL
	STB_PERLIN_IMPLEMENTATION
	STB_IMAGE_IMPLEMENTATION
	SOL_ALL_SAFETIES_ON=1
	SOL_NOEXCEPT_FUNCTION_TYPE=1
)

