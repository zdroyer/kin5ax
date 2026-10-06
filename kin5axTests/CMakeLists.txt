# The GoogleTest suite of kin5ax. GoogleTest comes through FetchContent (a network connection the first
# time); the test chains are copied next to the executable, where TestData.cpp expects them.
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)   # the same runtime as kin5ax on MSVC
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(kin5axTests
  main.cpp
  TestData.cpp
  SolverRoundTrip.cpp
  CLDataParsingTests.cpp
  CLDataTests.cpp
  ChainFcmTests.cpp
  ChainFormatPathsTests.cpp
  ChainJsonTests.cpp
  ChainParameterTests.cpp
  ChainTests.cpp
  ChainToolCollisionTests.cpp
  ChainXmlReaderTests.cpp
  DoubleToolsTests.cpp
  ExceptionsTests.cpp
  InterpolationTests.cpp
  LocaleTests.cpp
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
