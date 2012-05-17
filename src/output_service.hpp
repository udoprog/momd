#ifndef __OUTPUT_SERVICE__
#define __OUTPUT_SERVICE__

#include "messages.hpp"
#include "output.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

class output_service
{
public:
    output_service(zmq::context_t* context, output_base_ptr output);
    ~output_service();

    void run();
private:
    void send_status(frame::OutputStatusType status_type);

    void kill(frame::frame_container& container);
    void output_request_format(frame::frame_container& container);

    void recv_decoder_data();
    void loop(zmq::pollitem_t items[], size_t length);

    zmq::socket_t management;
    zmq::socket_t output_data;
    zmq::socket_t logger;

    output_base_ptr output;

    frame::handle_map handlers;

    bool stopped;
};

#endif /*  __OUTPUT_SERVICE__ */
