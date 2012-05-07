set(pulse_SOURCES pulse.cpp)
set(pulse_LIBRARIES pulse-simple)

add_library(pulse-plugin SHARED ${pulse_SOURCES})
target_link_libraries(pulse-plugin ${pulse_LIBRARIES})

set_target_properties(pulse-plugin PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
