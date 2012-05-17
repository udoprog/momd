cmake_minimum_required(VERSION 2.6)
project(momd)

set(CXXFLAGS "-O3 -Wall -pedantic -std=c++11")
set(CXXFLAGS_DEBUG "-O3 -Wall -pedantic -std=c++11 -g")
option(BUILD_DEV_LIBS "Attempt to build libraries under ./libs" ON)

set(momd_SOURCES 
  src/decoder_service.cpp
  src/dlopen.cpp
  src/input.cpp
  src/input_error.cpp
  src/log.cpp
  src/main.cpp
  src/main_service.cpp
  src/medialib_service.cpp
  src/messages.cpp
  src/output_base.cpp
  src/output.cpp
  src/output_error.cpp
  src/output_service.cpp
  src/logger_service.cpp
  src/pcm_format.cpp
  src/pcm_packet.cpp
  src/plugin.cpp
  src/audio_type.cpp
  )

find_package(Boost 1.46 COMPONENTS filesystem system REQUIRED)

set(momd_LIBRARIES
  msgpack
  zmq
  config++
  dl
  ${Boost_LIBRARIES}
  )

include_directories("${CMAKE_SOURCE_DIR}/libs/unc/src")
add_subdirectory(libs/unc)
#set(momd_LIBRARIES ${momd_LIBRARIES})

include_directories(${Boost_INCLUDE_DIR})
include_directories(src)

add_executable(momd ${momd_SOURCES})
add_executable(momd-debug EXCLUDE_FROM_ALL ${momd_SOURCES})

set_target_properties(momd PROPERTIES COMPILE_FLAGS ${CXXFLAGS})
set_target_properties(momd-debug PROPERTIES COMPILE_FLAGS ${CXXFLAGS_DEBUG})

target_link_libraries(momd ${momd_LIBRARIES})
target_link_libraries(momd-debug ${momd_LIBRARIES})

add_subdirectory(plugins)
