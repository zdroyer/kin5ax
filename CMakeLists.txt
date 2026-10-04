# kin5ax: the inverse kinematics library and its GoogleTest suite, on any platform with a C++17 compiler.
# The MFC application has its own Visual Studio solution in Universal5CMillingMachinePostprocessor/.
#
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   ctest --test-dir build --output-on-failure
cmake_minimum_required(VERSION 3.20)
project(kin5ax VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KIN5AX_BUILD_TESTS "Build the GoogleTest suite (kin5axTests)" ON)

add_subdirectory(kin5ax)

if(KIN5AX_BUILD_TESTS)
  enable_testing()
  add_subdirectory(kin5axTests)
endif()
