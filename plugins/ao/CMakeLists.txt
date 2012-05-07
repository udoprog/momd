set(ao_SOURCES ao.cpp)
set(ao_LIBRARIES ao)

add_library(ao-plugin SHARED ${ao_SOURCES})
target_link_libraries(ao-plugin ${ao_LIBRARIES})

set_target_properties(ao-plugin PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
