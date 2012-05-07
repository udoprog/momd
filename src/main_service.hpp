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
    std::string receive_identity();
    void handle_playing();
    void handle_panic();

    /* frame handlers */
    void ping(frame::frame_container& container);
    void kill(frame::frame_container& container);
    void play(frame::frame_container& container);
    void pause(frame::frame_container& container);
    void request_status(frame::frame_container& container);
    void decoder_error(frame::frame_container& container);
    void decoder_status(frame::frame_container& container);
    void output_status(frame::frame_container& container);
    void medialib_next_song(frame::frame_container& container);
    void medialib_status(frame::frame_container& container);

    /* loop helper */
    void loop(zmq::pollitem_t items[], size_t length);

    zmq::socket_t management;
    zmq::socket_t services;
    zmq::socket_t decoder_data;
    zmq::socket_t output_data;
    zmq::socket_t logger;
    frame::handle_map management_handlers;
    frame::handle_map service_handlers;

    /* service state */

    /* Decoder is ready to receive instructions.
     */
    bool decoder_ready;

    /*
     * Decoder has exited.
     */
    bool decoder_exit;

    /*
     * Decoder is busy actively decoding something.
     */
    bool decoder_busy;

    /*
     * Next song has been sent to decoder.
     */
    bool decoder_pending_next_song;

    /*
     * Output is ready to receive frames.
     */
    bool output_ready;

    /*
     * Output has exited.
     */
    bool output_exit;

    /*
     * Number of pending frames in output.
     */
    int output_pending_frames;

    /*
     * Medialib is ready to receive requests.
     */
    bool medialib_ready;

    /*
     * Medialib has exited.
     */
    bool medialib_exit;

    /*
     * A next song request is pending.
     */
    bool medialib_pending_next_song;

    /*
     * Is currently playing.
     */
    bool playing;

    /*
     * Service is stopped (shutdown).
     */
    bool stopped;

    /*
     * Any services has panicked.
     */
    bool any_service_panic;
};

#endif /*  __MAIN_SERVICE__ */
