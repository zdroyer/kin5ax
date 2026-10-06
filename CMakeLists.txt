# kin5ax: the inverse kinematics library and its GoogleTest suite, on any platform with a C++17 compiler.
# The MFC application has its own Visual Studio solution in Universal5CMillingMachinePostprocessor/.
#
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   ctest --test-dir build --output-on-failure
cmake_minimum_required(VERSION 3.20)
project(kin5ax VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KIN5AX_BUILD_TESTS "Build the GoogleTest suites" ON)
option(KIN5AX_BUILD_C_ABI "Build the C interface of the library (kin5axc)" ON)

add_subdirectory(kin5ax)

if(KIN5AX_BUILD_TESTS)
  # GoogleTest for both suites, fetched once (a network connection the first time)
  enable_testing()
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
  add_subdirectory(kin5axTests)
endif()

if(KIN5AX_BUILD_C_ABI)
  add_subdirectory(kin5axc)
  if(KIN5AX_BUILD_TESTS)
    add_subdirectory(kin5axcTests)
  endif()
endif()
