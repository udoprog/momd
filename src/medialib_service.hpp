#ifndef __MEDIALIB_EVENT_LOOP__
#define __MEDIALIB_EVENT_LOOP__

#include "messages.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

class medialib_service
{
public:
    medialib_service(zmq::context_t* context);
    ~medialib_service();

    void add_song(std::string path);

    void run();
private:
    void send_status(frame::MedialibStatusType status_type);

    void kill(frame::frame_container& container);
    void request_next(frame::frame_container& container);

    void loop(zmq::pollitem_t items[], size_t length);

    zmq::socket_t logger;
    zmq::socket_t management;
    frame::handle_map handlers;

    std::vector<std::string> songs;
    unsigned int next_song;
    bool killed;
};

#endif /*  __MEDIALIB_EVENT_LOOP__ */
