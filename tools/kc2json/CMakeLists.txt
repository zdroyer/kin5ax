# The way out for a file still in the retired XML form: reads .kc, writes .kc.json.
add_executable(kc2json kc2json.cpp)
target_link_libraries(kc2json PRIVATE kin5ax)
