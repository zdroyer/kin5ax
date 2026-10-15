# kin5axc: the C interface of kin5ax as a shared library. C++ inside, C outside - nothing of the C++
# library is exported, so a host can be built with any compiler and any runtime.
add_library(kin5axc SHARED
  src/kin5axc.cpp
  src/kin5axfcm.cpp        # the FreeCAD machine file bridge, a header of its own over the same library
)

target_include_directories(kin5axc
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# the static core, linked in whole: the C interface is the only way in
target_link_libraries(kin5axc PRIVATE kin5ax::kin5ax)

# K5_API exports while this library is built and imports for whoever includes the header afterwards
target_compile_definitions(kin5axc PRIVATE KIN5AXC_BUILDING)

set_target_properties(kin5axc PROPERTIES
  CXX_VISIBILITY_PRESET hidden       # only the k5_* functions leave the shared object
  VISIBILITY_INLINES_HIDDEN ON
  POSITION_INDEPENDENT_CODE ON
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

# and on a GNU-like linker the standard library's own instantiations are shut in as well
if(UNIX AND NOT APPLE)
  target_link_options(kin5axc PRIVATE
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/kin5axc.map"
    "-Wl,--exclude-libs,ALL"
  )
endif()

target_compile_options(kin5axc PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wno-overloaded-virtual>
)

# What a release archive holds. The static core is deliberately not installed: the C interface is the
# only way in, and shipping kin5ax.lib beside it would invite linking against C++ classes that are
# part of no promise and that change between any two versions.
include(GNUInstallDirs)
install(TARGETS kin5axc
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}        # kin5axc.dll on Windows
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}        # libkin5axc.so and its links on Linux
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})       # kin5axc.lib, the import library of the dll
install(FILES include/kin5ax.h include/kin5axfcm.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# and a test that says so, because this is the kind of promise that rots quietly
if(UNIX AND NOT APPLE AND KIN5AX_BUILD_TESTS)
  add_test(NAME kin5axcExports COMMAND ${CMAKE_COMMAND}
    -DLIB=$<TARGET_FILE:kin5axc> -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckExports.cmake)
endif()
