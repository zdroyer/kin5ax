# The way out for a file still in the retired XML form: reads .kc, writes .kc.json.
# The XML reader lives here now, and nowhere else: this is the only thing that reads a .kc.
add_executable(kc2json kc2json.cpp ChainXmlReader.cpp)
target_include_directories(kc2json PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kc2json PRIVATE kin5ax)
