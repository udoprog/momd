#include "logger_service.hpp"

#include <iostream>

#define LOGGER_INPUT(items) items[0]

logger_service::logger_service(zmq::context_t* context)
    : logger_input(*context, ZMQ_PULL),
      stopped(false)
{
    logger_input.bind("inproc://logger");
}

void logger_service::exhaust_log() {
    zmq::pollitem_t items[] = {
        {logger_input, 0, ZMQ_POLLIN, 0}
    };

    while (true) {
        zmq::poll(items, 1, 1000);

        if (items[0].revents & ZMQ_POLLIN) {
            receive_and_print_log();
            continue;
        }

        break;
    }
}

void logger_service::run()
{
    zmq::pollitem_t items[] = {
        {logger_input, 0, ZMQ_POLLIN, 0}
    };

    while (!stopped) {
        loop(items, sizeof(items) / sizeof(zmq::pollitem_t));
    }
}

void logger_service::loop(zmq::pollitem_t items[], size_t length)
{
    zmq::poll(items, length, -1);

    if (LOGGER_INPUT(items).revents & ZMQ_POLLIN) {
        receive_and_print_log();
    }
}

void logger_service::stop()
{
    stopped = true;
}

void logger_service::receive_and_print_log()
{
    zmq::message_t message;
    logger_input.recv(&message);
    std::string log_message((const char*)message.data(), message.size());
    std::cout << "LOG: " << log_message << std::endl;
}

logger_service::~logger_service()
{
    logger_input.close();
    exhaust_log();
}
