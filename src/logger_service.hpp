#ifndef __LOGGER_SERVICE__
#define __LOGGER_SERVICE__

#include "messages.hpp"
#include "input.hpp"

#include <zmq.hpp>

class logger_service
{
public:
    logger_service(zmq::context_t* context);
    ~logger_service();

    void run();
    void stop();
private:
    void loop(zmq::pollitem_t items[], size_t length);

    /* log handling functions */
    void receive_and_print_log();
    void exhaust_log();

    zmq::socket_t logger_input;
    bool stopped;
};


#endif /*  __LOGGER_SERVICE__ */
