#include "log.hpp"

#include <cstdarg>
#include <iostream>

void log_remote(zmq::socket_t& socket, int level, const char* fmt, ...)
{
    char log_buffer[BUFFER_SIZE];
    size_t n;

    ::va_list ap;

    ::va_start(ap, fmt);
        n = vsnprintf(log_buffer, BUFFER_SIZE, fmt, ap);
    ::va_end(ap);

    zmq::message_t log_message(n);
    ::memcpy(log_message.data(), log_buffer, strlen(log_buffer));

    try {
        socket.send(log_message);
    } catch (...) {
        /* catch any exceptions occuring */
        std::cerr << "LOG: INTERNAL SOCKET FAILURE" << std::endl;
    }
}
