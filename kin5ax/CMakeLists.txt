# The kin5ax library: standard library only; pugixml and nlohmann/json are vendored under 3rdParty/.
# Mirrors kin5ax.vcxproj, which the Visual Studio solution of the MFC application still uses.
add_library(kin5ax STATIC
  3DGeometry/M4x4.cpp
  3DGeometry/R3Box.cpp
  3DGeometry/R3AnchoredVector.cpp
  3DGeometry/R3Point.cpp
  3DGeometry/R3Vector.cpp
  3DGeometry/Rotation.cpp
  3DGeometry/Scale.cpp
  3DGeometry/Transformation.cpp
  3DGeometry/Translation.cpp
  Exceptions/GeneratorException.cpp
  Chain/Chain.cpp
  Chain/ChainConfiguration.cpp
  Chain/CollisionReport.cpp
  Chain/ChainSegment.cpp
  Chain/ChainJson.cpp
  Chain/ChainFcm.cpp
  Chain/ChainFileReader.cpp
  MillingTool/MillingTool.cpp
  MillingTool/ToolHolder.cpp
  MillingTool/MillingToolBall.cpp
  MillingTool/MillingToolCylinder.cpp
  Postprocessor/5CPostprocessor.cpp
  Postprocessor/OrientationPostprocessor.cpp
  Postprocessor/PositionPostprocessor.cpp
  Postprocessor/SolveReport.cpp
  Simulation/CuttingSimulation.cpp
  Simulation/Material.cpp
  Simulation/MaterialSimulation.cpp
  Simulation/ToolPathSimulation.cpp
  Simulation/SphereCLDataGenerator.cpp
  Simulation/SimpleSimulation.cpp
  CLDataPostprocessor.cpp
  CutterLocationData.cpp
  LocationInterpolation.cpp
  PostprocessorSettings.cpp
  SimulationData.cpp
  TrajectoryStart.cpp
  Version.cpp
  string_tools.cpp
  3rdParty/pugixml/pugixml.cpp
)
add_library(kin5ax::kin5ax ALIAS kin5ax)

# what Version.cpp answers CommitHash() with; the root CMakeLists worked it out from git
target_compile_definitions(kin5ax PRIVATE KIN5AX_COMMIT="${KIN5AX_COMMIT}")

# the sources include each other by bare file name; the consumers (tests, application) do the same
target_include_directories(kin5ax PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions
  ${CMAKE_CURRENT_SOURCE_DIR}/3DGeometry
  ${CMAKE_CURRENT_SOURCE_DIR}/Chain
  ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor
  ${CMAKE_CURRENT_SOURCE_DIR}/MillingTool
  ${CMAKE_CURRENT_SOURCE_DIR}/Simulation
)
target_compile_features(kin5ax PUBLIC cxx_std_17)
# The static library goes into the shared C interface (kin5axc) as well: a shared object may only be
# built out of position-independent code, and nothing of this library may leave that shared object -
# only its k5_* functions are exported, so these objects are compiled hidden. On Windows, where a
# symbol is exported only when it says so, both lines say nothing.
set_target_properties(kin5ax PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
)
target_compile_options(kin5ax PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
  # -Wno-overloaded-virtual: CTransformation::operator* hides the virtual CM4x4::operator*(const CM4x4&);
  # a design item of the geometry classes (KnownBugs #69), not something to paper over with a using-declaration
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wno-overloaded-virtual>
)
