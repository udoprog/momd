cmake_minimum_required(VERSION 2.6)
project(momd)

set(CXXFLAGS "-O3 -Wall -pedantic -std=c++11")
set(CXXFLAGS_DEBUG "-O3 -Wall -pedantic -std=c++11 -g")

set(momd_SOURCES 
  src/output_base.cpp
  src/output_error.cpp
  src/main.cpp
  src/main_service.cpp
  src/decoder_service.cpp
  src/output_service.cpp
  src/medialib_service.cpp
  src/messages.cpp
  src/output.cpp
  src/input.cpp
  src/plugin.cpp
  src/io.cpp
  src/dlopen.cpp
  )

find_package(Boost 1.46 COMPONENTS filesystem system REQUIRED)

set(momd_LIBRARIES
  msgpack
  zmq
  config++
  dl
  ${Boost_LIBRARIES}
  )

include_directories(${Boost_INCLUDE_DIR})
include_directories(src)

add_executable(momd ${momd_SOURCES})
add_executable(momd-debug EXCLUDE_FROM_ALL ${momd_SOURCES})

set_target_properties(momd PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
set_target_properties(momd-debug PROPERTIES COMPILE_FLAGS ${CXXFLAGS_DEBUG})

target_link_libraries(momd msgpack ${momd_LIBRARIES})
target_link_libraries(momd-debug msgpack ${momd_LIBRARIES})

add_subdirectory(plugins)
