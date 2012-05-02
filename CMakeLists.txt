cmake_minimum_required(VERSION 2.6)
project(mpdpp)

set(mpdpp_SOURCES 
  src/main.cpp
  src/main_event_loop.cpp
  src/decoder_event_loop.cpp
  src/output_event_loop.cpp
  src/messages.cpp
  src/output.cpp
  src/output/pulse.cpp
  src/input/mpg123.cpp
  src/input/vorbis.cpp
  src/input/flac.cpp
  src/input.cpp
  src/io.cpp
  )

set(mpdpp_LIBRARIES
  msgpack
  zmq
  boost_system
  boost_filesystem
  config++
  vorbisfile
  FLAC++
  mpg123
  pulse-simple
  )

include_directories(src)

add_executable(mpd++ ${mpdpp_SOURCES})
add_executable(mpd++-debug ${mpdpp_SOURCES})

set_target_properties(mpd++ PROPERTIES COMPILE_FLAGS "-O3 -Wall -pedantic -std=c++11")
set_target_properties(mpd++-debug PROPERTIES COMPILE_FLAGS "-O0 -Wall -pedantic -std=c++11 -g")

target_link_libraries(mpd++ msgpack ${mpdpp_LIBRARIES})
target_link_libraries(mpd++-debug msgpack ${mpdpp_LIBRARIES})
