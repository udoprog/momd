#include "output_event_loop.hpp"

#include "output.hpp"
#include "io.hpp"

#include "messages.hpp"

void send_output_status(zmq::socket_t& socket, OutputStatusType status_type)
{
    OutputStatus status(status_type);
    send_frame(socket, status);
}

void recv_decoder_data(output_base_ptr output, zmq::socket_t& socket)
{
    zmq::message_t message;
    socket.recv(&message);

    std::cerr << "output: decoder data (" << message.size() << " bytes)" << std::endl;

    pcm_packet_ptr pcm(
            new pcm_packet((const char*)message.data(), message.size()));

    if (output) {
        output->write(pcm);
    }
}

#define OUTPUT_DATA(items) items[0]

void output_event_loop(zmq::context_t* context, libconfig::Config* config)
{
    output_base_ptr output;

    if (config->exists("output")) {
        libconfig::Setting& output_config = config->lookup("output");

        std::string type;

        if (output_config.lookupValue("type", type)) {
            output = open_output(type);
            output->setup(output_config);
            output->open();
        }
    }

    zmq::socket_t output_data(*context, ZMQ_PULL);
    output_data.connect("inproc://output/data");

    zmq::socket_t output_status(*context, ZMQ_PUSH);
    output_status.connect("inproc://output/status");

    zmq::pollitem_t items[] = {
        {output_data, 0, ZMQ_POLLIN, 0}
    };

    while (true) {
        zmq::poll(items, 1, -1);

        if (OUTPUT_DATA(items).revents & ZMQ_POLLIN) {
            recv_decoder_data(output, output_data);
            send_output_status(output_status, FrameReceived);
        }
    }
}
