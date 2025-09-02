add_library(core STATIC
   src/Bom.cpp
   src/Boolean.cpp
   src/Draft.cpp
   src/Export.cpp
   src/Extrude.cpp
   src/Features.cpp
   src/Gear.cpp
   src/LuaEngine.cpp
   src/Measure.cpp
   src/Primitives.cpp
   src/Project.cpp
   src/Sketch.cpp
   src/CoarseThread.cpp
   src/Thread.cpp
   src/ThreadChamfer.cpp
   src/Transform.cpp
   src/Triangulate.cpp
   src/lua/BindBooleans.cpp
   src/lua/BindConstruct.cpp
   src/lua/BindDraft.cpp
   src/lua/BindFeatures.cpp
   src/lua/BindGears.cpp
   src/lua/BindIO.cpp
   src/lua/BindMeasure.cpp
   src/lua/BindPrimitives.cpp
   src/lua/BindSketch.cpp
   src/lua/BindThreads.cpp
   src/lua/BindTransforms.cpp
   src/lua/BindingUtils.cpp
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
target_link_libraries(core PRIVATE ${OCCT_LIBS})

if(TARGET OpenCASCADE::TKSTL)
  target_link_libraries(core PRIVATE OpenCASCADE::TKSTL)
endif()
if(TARGET OpenCASCADE::TKXDESTEP)
  target_link_libraries(core PRIVATE OpenCASCADE::TKXDESTEP)
endif()

# --- now configure the target ---
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/core
    ${LUA_INCLUDE_DIR}
    ${sol2_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(core PRIVATE
    project_settings
    nlohmann_json::nlohmann_json
    ${LUA_LIBRARIES}
	PUBLIC glm::glm
)
