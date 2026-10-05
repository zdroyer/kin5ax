# The kin5ax library: standard library only; pugixml and nlohmann/json are vendored under 3rdParty/.
# Mirrors kin5ax.vcxproj, which the Visual Studio solution of the MFC application still uses.
add_library(kin5ax STATIC
  3DGeometry/M4x4.cpp
  3DGeometry/R3Box.cpp
  3DGeometry/R3CaughtVector.cpp
  3DGeometry/R3Point.cpp
  3DGeometry/R3Vector.cpp
  3DGeometry/Rotation.cpp
  3DGeometry/Scale.cpp
  3DGeometry/Transformation.cpp
  3DGeometry/Translation.cpp
  Exceptions/GeneratorException.cpp
  Chain/Chain.cpp
  Chain/ChainConfiguration.cpp
  Chain/ChainSegment.cpp
  Chain/ChainXmlReader.cpp
  Chain/ChainJson.cpp
  Chain/ChainFcm.cpp
  Chain/ChainFileReader.cpp
  MillingTool/MillingTool.cpp
  MillingTool/MillingToolBall.cpp
  MillingTool/MillingToolCylinder.cpp
  Postprocessor/5CPostprocessor.cpp
  Postprocessor/OrientationPostprocessor.cpp
  Postprocessor/PositionPostprocessor.cpp
  Postprocessor/SolveReport.cpp
  Simulation/Material.cpp
  Simulation/MaterialSimulation.cpp
  Simulation/SphereCLDataGenerator.cpp
  Simulation/SimpleSimulation.cpp
  CLDataPostprocessor.cpp
  CutterLocationData.cpp
  PostprocessorSettings.cpp
  SimulationData.cpp
  string_tools.cpp
  3rdParty/pugixml/pugixml.cpp
)
add_library(kin5ax::kin5ax ALIAS kin5ax)

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
target_compile_options(kin5ax PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
  # -Wno-overloaded-virtual: CTransformation::operator* hides the virtual CM4x4::operator*(const CM4x4&);
  # a design item of the geometry classes (KnownBugs #69), not something to paper over with a using-declaration
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wno-overloaded-virtual>
)
