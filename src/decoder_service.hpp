#ifndef __DECODER_SERVICE__
#define __DECODER_SERVICE__

#include "messages.hpp"
#include "input.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

class decoder_service
{
public:
    decoder_service(zmq::context_t* context);
    ~decoder_service();

    void run();
private:
    void kill(frame::frame_container& container);
    void next_song(frame::frame_container& container);
    void decoder_request_frame(frame::frame_container& container);
    void decoder_initialize(frame::frame_container& container);
    void send_status(frame::DecoderStatusType status_type);

    void loop(zmq::pollitem_t items[], size_t length);

    input_base_ptr input;
    zmq::socket_t management;
    zmq::socket_t decoder_data;
    zmq::socket_t logger;
    frame::handle_map handlers;
    bool stopped;
};


#endif /*  __DECODER_SERVICE__ */
