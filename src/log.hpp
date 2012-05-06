#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <algorithm>
#include <zmq.hpp>

#define LOG_ERROR 1
#define LOG_INFO 3
#define LOG_DEBUG 5

#ifndef LOG_LEVEL
#  define LOG_LEVEL LOG_DEBUG
#endif

#include <cstdio>

#define LOG(socket, level, ...)\
    if (level <= LOG_LEVEL) { log_remote(socket, level,  __VA_ARGS__); }

#define BUFFER_SIZE 1024

void log_remote(zmq::socket_t& socket, int level, const char* fmt, ...);

#endif /* __LOG_HPP__ */
