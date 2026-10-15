# The tests of the C interface. They link kin5axc and nothing else: if a host needed the C++ library as
# well, this target would not link, which is exactly what it is here to prove.
enable_language(C)

add_executable(kin5axcTests
  main.cpp
  AbiTests.cpp
  FcmAbiTests.cpp
  CHeaderCheck.c      # the public header, compiled by the C compiler
)

target_link_libraries(kin5axcTests PRIVATE kin5axc GTest::gtest)

# the machine models of the C++ suite, read straight from the source tree
target_compile_definitions(kin5axcTests PRIVATE
  KIN5AXC_TEST_CHAINS_DIR="${CMAKE_SOURCE_DIR}/kin5axTests/TestChains"
  # the tests write their cutter-location files with fopen, which MSVC calls unsafe
  $<$<OR:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>:_CRT_SECURE_NO_WARNINGS>
)

target_compile_options(kin5axcTests PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W3>>
  $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>>
  $<$<COMPILE_LANGUAGE:C>:$<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wpedantic>>
)

# the shared library has to be found at run time, which on Windows means next to the executable
add_custom_command(TARGET kin5axcTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:kin5axc> $<TARGET_FILE_DIR:kin5axcTests>
  COMMENT "Copying kin5axc next to kin5axcTests"
)

add_test(NAME kin5axcTests COMMAND kin5axcTests)
