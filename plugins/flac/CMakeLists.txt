set(flac_SOURCES flac.cpp)
set(flac_LIBRARIES FLAC++)

add_library(flac-plugin SHARED ${flac_SOURCES})
target_link_libraries(flac-plugin ${flac_LIBRARIES})

set_target_properties(flac-plugin PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
