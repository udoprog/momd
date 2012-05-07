set(vorbis_SOURCES vorbis.cpp)
set(vorbis_LIBRARIES vorbisfile)

add_library(vorbis-plugin SHARED ${vorbis_SOURCES})
target_link_libraries(vorbis-plugin ${vorbis_LIBRARIES})

set_target_properties(vorbis-plugin PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
