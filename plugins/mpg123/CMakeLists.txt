set(mpg123_SOURCES mpg123.cpp)
set(mpg123_LIBRARIES unc uncdata mpg123)

add_library(mpg123-plugin SHARED ${mpg123_SOURCES})
target_link_libraries(mpg123-plugin ${mpg123_LIBRARIES})

set_target_properties(mpg123-plugin PROPERTIES COMPILE_FLAGS "${CXXFLAGS}")
