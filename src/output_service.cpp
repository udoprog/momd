#include "output_service.hpp"

#include "log.hpp"
#include "messages.hpp"
#include "output.hpp"
#include "output_base.hpp"
#include "pcm_packet.hpp"

#include <functional>

#define OUTPUT_DATA(items) items[0]
#define MANAGEMENT(items) items[1]

output_service::output_service(zmq::context_t* context, output_base_ptr output)
    : management(*context, ZMQ_DEALER),
      output_data(*context, ZMQ_PULL),
      logger(*context, ZMQ_PUSH),
      output(output),
      handlers(),
      stopped(false)
{
    using namespace std::placeholders;
    using namespace frame;
    using std::bind;

    management.setsockopt(ZMQ_IDENTITY, "output", 6);
    management.connect("inproc://services");

    output_data.connect("inproc://output/data");
    logger.connect("inproc://logger");

    handlers[Kill::TYPE]
        = bind(&output_service::kill, this, _1);
}

output_service::~output_service()
{
    output_data.close();
    management.close();
    logger.close();
}

void output_service::kill(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "output: Kill")
    stopped = true;
}

void output_service::send_status(frame::OutputStatusType status_type)
{
    frame::OutputStatus status(status_type);
    send_frame(management, status);
}

void output_service::recv_decoder_data()
{
    zmq::message_t message;
    output_data.recv(&message);

    pcm_packet::ptr pcm(
            new pcm_packet((const char*)message.data(), message.size()));

    if (output) {
        output->write(pcm);
    }
}

void output_service::run()
{
    zmq::pollitem_t items[] = {
        {output_data, 0, ZMQ_POLLIN, 0},
        {management, 0, ZMQ_POLLIN, 0}
    };

    size_t items_size = sizeof(items) / sizeof(zmq::pollitem_t);

    try {
        send_status(frame::OutputReady);

        while (!stopped) {
            loop(items, items_size);
        }
    } catch (std::exception& e) {
        LOG(logger, LOG_ERROR, "output: Error: %s", e.what());
        send_status(frame::OutputPanic);
    }

    LOG(logger, LOG_INFO, "output: Closing");

    send_status(frame::OutputExit);
}

void output_service::loop(zmq::pollitem_t items[], size_t length)
{
    zmq::poll(items, length, -1);

    if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(management, handlers);
    }

    if (OUTPUT_DATA(items).revents & ZMQ_POLLIN) {
        recv_decoder_data();
        send_status(frame::OutputFrameReceived);
    }
}
