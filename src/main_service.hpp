#ifndef __MAIN_SERVICE__
#define __MAIN_SERVICE__

#include "messages.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

class main_service
{
public:
    main_service(zmq::context_t& context);
    ~main_service();

    void run();
private:
    /* frame handlers */
    void ping(frame::frame_container& container);
    void kill(frame::frame_container& container);
    void play(frame::frame_container& container);
    void pause(frame::frame_container& container);
    void request_status(frame::frame_container& container);
    void decoder_error(frame::frame_container& container);
    void decoder_status(frame::frame_container& container);
    void output_status(frame::frame_container& container);
    void medialib_next(frame::frame_container& container);
    void medialib_status(frame::frame_container& container);

    /* log handling functions */
    void receive_and_print_log();
    void exhaust_log();

    /* loop helper */
    void loop(zmq::pollitem_t items[], size_t length);

    zmq::socket_t management;
    zmq::socket_t decoder;
    zmq::socket_t medialib;
    zmq::socket_t output;
    zmq::socket_t decoder_data;
    zmq::socket_t output_data;
    zmq::socket_t logger_input;
    zmq::socket_t logger;
    frame::handle_map handlers;

    /* service state */
    int decoder_requested_frames;
    bool decoder_ready;
    bool playing;
    bool killed;

    bool decoder_exit;
    bool output_exit;
    bool medialib_exit;
};

#endif /*  __MAIN_SERVICE__ */
