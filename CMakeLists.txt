# kin5ax: the inverse kinematics library and its GoogleTest suite, on any platform with a C++17 compiler.
# The MFC application has its own Visual Studio solution in kin5axChainEditorPlus/.
#
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build
#   ctest --test-dir build --output-on-failure
cmake_minimum_required(VERSION 3.20)

# The version lives in kin5ax/Version.h, which the Visual Studio build of the MFC application
# compiles as well, so it is read from there rather than written twice.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/kin5ax/Version.h" KIN5AX_VERSION_HEADER)
foreach(part MAJOR MINOR PATCH)
  if(NOT KIN5AX_VERSION_HEADER MATCHES "KIN5AX_VERSION_${part}[ \t]+([0-9]+)")
    message(FATAL_ERROR "kin5ax/Version.h does not define KIN5AX_VERSION_${part}")
  endif()
  set(KIN5AX_VERSION_${part} ${CMAKE_MATCH_1})
endforeach()

project(kin5ax
  VERSION ${KIN5AX_VERSION_MAJOR}.${KIN5AX_VERSION_MINOR}.${KIN5AX_VERSION_PATCH}
  LANGUAGES CXX C)

# The commit the library is built from, compiled into it so that a shipped binary can be traced back
# to its source - kin5axc travels inside the FreeCAD addon, where nothing else says where it came
# from. This is build metadata in the semantic-versioning sense: it takes no part in the version a
# host checks for compatibility, which is why it is kept out of KIN5AX_VERSION_STRING.
set(KIN5AX_COMMIT "unknown")
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=12 HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE KIN5AX_GIT_HEAD OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET RESULT_VARIABLE KIN5AX_GIT_RESULT)
  if(KIN5AX_GIT_RESULT EQUAL 0 AND NOT KIN5AX_GIT_HEAD STREQUAL "")
    set(KIN5AX_COMMIT "${KIN5AX_GIT_HEAD}")
    # a tracked file changed and not committed: the binary is not that commit and should not claim to be
    execute_process(COMMAND ${GIT_EXECUTABLE} status --porcelain --untracked-files=no
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE KIN5AX_GIT_DIRTY OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(NOT KIN5AX_GIT_DIRTY STREQUAL "")
      set(KIN5AX_COMMIT "${KIN5AX_COMMIT}-dirty")
    endif()
  endif()
  # committing changes the hash, and nothing else would make CMake look again
  foreach(KIN5AX_GIT_FILE HEAD index)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git/${KIN5AX_GIT_FILE}")
      set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/.git/${KIN5AX_GIT_FILE}")
    endif()
  endforeach()
endif()
message(STATUS "kin5ax ${PROJECT_VERSION}+${KIN5AX_COMMIT}")

# The same thing a reader can see without loading the library: which commit the binaries beside this
# file were built from. An archive that has been unpacked, renamed and passed on still says.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/BUILD-INFO.txt"
"kin5ax ${PROJECT_VERSION}+${KIN5AX_COMMIT}

version   ${PROJECT_VERSION}        what a host checks for compatibility (k5_version)
commit    ${KIN5AX_COMMIT}        the source these binaries were built from (k5_commit_hash)
system    ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}
compiler  ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}

The library answers with all of this at run time: k5_build_string() returns
\"${PROJECT_VERSION}+${KIN5AX_COMMIT}\". A \"-dirty\" suffix means the working tree held changes that
are in no commit, so the binary is not exactly that commit. \"unknown\" means it was built outside a
git checkout.

https://github.com/zdroyer/kin5ax
")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(KIN5AX_BUILD_TESTS "Build the GoogleTest suites" ON)
option(KIN5AX_BUILD_C_ABI "Build the C interface of the library (kin5axc)" ON)

add_subdirectory(kin5ax)

# the one-way converter for files still in the retired XML form
add_subdirectory(tools/kc2json)

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

# The rest of a release archive: what the licence requires and what a reader needs to know. The
# binaries and the header are installed by kin5axc/CMakeLists.txt.
if(KIN5AX_BUILD_C_ABI)
  install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_BINARY_DIR}/BUILD-INFO.txt"
    DESTINATION .)
endif()
