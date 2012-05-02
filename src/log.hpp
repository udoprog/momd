#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <zmq.hpp>

#define LOG_ERROR 1
#define LOG_INFO 3
#define LOG_DEBUG 5

#ifndef LOG_LEVEL
#  define LOG_LEVEL LOG_DEBUG
#endif

#include <cstdio>

#define LOG(socket, level, ...)\
    if (level <= LOG_LEVEL) {\
        char log_buffer[1025];\
        ::memset(log_buffer, '\x00', 1025);\
        ::snprintf(log_buffer, 1024, __VA_ARGS__);\
        zmq::message_t log_message(strlen(log_buffer));\
        ::memcpy(log_message.data(), log_buffer, strlen(log_buffer));\
        socket.send(log_message);\
    }

#endif /* __LOG_HPP__ */
