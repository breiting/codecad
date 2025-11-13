add_library(kernel STATIC
	src/Chamfer.cpp
	src/Curves.cpp
	src/CurvedPlate.cpp
	src/Dxf.cpp
	src/Export.cpp
	src/EdgeSelector.cpp
	src/Extrude.cpp
	src/Fillet.cpp
	src/Logger.cpp
	src/OcctShape.cpp
	src/Operations.cpp
	src/PipeAdapter.cpp
	src/Poisson.cpp
	src/PoissonDisk.cpp
	src/Primitives.cpp
	src/Rectangle.cpp
	src/Revolve.cpp
	src/Rod.cpp
	src/Section.cpp
	src/SketchProfiles.cpp
	src/Threads.cpp
	src/Triangulation.cpp
)

# --- OpenCascade handling for different plattforms
find_package(OpenCASCADE REQUIRED)

set(OCCT_WANTED
	TKernel TKMath TKG2d TKG3d
	TKGeomBase TKGeomAlgo
	TKBRep TKTopAlgo TKPrim TKBO TKShHealing
	TKMesh TKXMesh TKService
	TKSTL
	TKXDESTEP
)

set(OCCT_LIBS "")

foreach(name IN LISTS OCCT_WANTED)
	if(TARGET OpenCASCADE::${name})
		list(APPEND OCCT_LIBS OpenCASCADE::${name})
	endif()
endforeach()

# Fallback (for MacOS)
if(OCCT_LIBS STREQUAL "")
	set(OCCT_LIBS ${OpenCASCADE_LIBRARIES})

	# Do not use DRAW/Test/Tcl/Tk
	list(FILTER OCCT_LIBS EXCLUDE REGEX "Draw|DCAF|XSDRAW|TopTest|ViewerTest|QADraw|TObjDRAW|XDEDRAW|TKTObj")
endif()

message(STATUS "OpenCASCADE: ${OpenCASCADE_VERSION}")


# 4) Linken
target_link_libraries(kernel PRIVATE ${OCCT_LIBS})

if(TARGET OpenCASCADE::TKSTL)
	target_link_libraries(kernel PRIVATE OpenCASCADE::TKSTL)
endif()
if(TARGET OpenCASCADE::TKXDESTEP)
	target_link_libraries(kernel PRIVATE OpenCASCADE::TKXDESTEP)
endif()

# --- now configure the target ---
target_include_directories(kernel PUBLIC
				${CMAKE_CURRENT_SOURCE_DIR}/inc
				${CMAKE_CURRENT_SOURCE_DIR}/private
				${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(kernel PRIVATE
				project_settings
				nlohmann_json::nlohmann_json
	PUBLIC glm::glm
)

add_subdirectory(tests)
