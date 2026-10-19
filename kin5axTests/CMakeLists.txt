# The GoogleTest suite of kin5ax. GoogleTest is fetched by the root CMakeLists, for this suite and for
# the one of the C interface; the test chains are copied next to the executable, where TestData.cpp
# expects them.

add_executable(kin5axTests
  main.cpp
  TestData.cpp
  SolverRoundTrip.cpp
  CLDataInMemoryTests.cpp
  CLDataParsingTests.cpp
  CLDataTests.cpp
  ChainFcmTests.cpp
  ChainFormatPathsTests.cpp
  ChainJsonTests.cpp
  ChainParameterTests.cpp
  ChainTests.cpp
  ChainToolCollisionTests.cpp
  CollisionReportTests.cpp
  DoubleToolsTests.cpp
  ExceptionsTests.cpp
  InterpolationTests.cpp
  LocaleTests.cpp
  LocationInterpolationTests.cpp
  M4x4Tests.cpp
  MaterialGougeTests.cpp
  MaterialSimulationTests.cpp
  MaterialTests.cpp
  MeridianCLDataTests.cpp
  MillingToolTests.cpp
  ProgressTests.cpp
  R3BoxTests.cpp
  RotationTests.cpp
  SegmentFrameTests.cpp
  SimpleSimulationTests.cpp
  SolvedLocationTests.cpp
  SimulationDataTests.cpp
  SolveReportTests.cpp
  StringToolsTests.cpp
  ToolAssemblyTests.cpp
  ToolPathSimulationTests.cpp
  TrajectoryStartTests.cpp
  VersionTests.cpp
  ToolMountTests.cpp
)
target_include_directories(kin5axTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kin5axTests PRIVATE kin5ax::kin5ax GTest::gtest)
target_compile_options(kin5axTests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wno-overloaded-virtual>
)

# the machine models the tests run on, next to the exe (TestData.cpp: GetTestChainsDir)
add_custom_command(TARGET kin5axTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_CURRENT_SOURCE_DIR}/TestChains $<TARGET_FILE_DIR:kin5axTests>/TestChains
  COMMENT "Copying the test chains next to kin5axTests"
)

add_test(NAME kin5axTests COMMAND kin5axTests)
