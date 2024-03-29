#include "medialib_service.hpp"

#include "messages.hpp"
#include "log.hpp"

#include <vector>
#include <stdexcept>

#include <functional>

#define MANAGEMENT(items) items[0]

medialib_service::medialib_service(zmq::context_t* context)
    : logger(*context, ZMQ_PUSH),
      management(*context, ZMQ_DEALER),
      handlers(),
      songs(),
      next_song(0),
      stopped(false)
{
    using namespace std::placeholders;
    using namespace frame;
    using std::bind;

    logger.connect("inproc://logger");

    management.setsockopt(ZMQ_IDENTITY, "medialib", 8);
    management.connect("inproc://services");

    handlers[Kill::TYPE] =
        bind(&medialib_service::kill, this, _1);

    handlers[MedialibRequestNextSong::TYPE] =
        bind(&medialib_service::request_next_song, this, _1);
}

medialib_service::~medialib_service() {
    logger.close();
    management.close();
}

void medialib_service::kill(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "medialib: Kill")

    stopped = true;
}

void medialib_service::request_next_song(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "medialib: MedialibRequestNextSong");

    frame::MedialibNextSong next;
    next.path = songs[next_song];
    send_frame(management, next);

    if (++next_song >= songs.size()) {
        next_song = 0;
    }
}

void medialib_service::send_status(frame::MedialibStatusType status_type) {
    frame::MedialibStatus status(status_type);
    send_frame(management, status);
}

void medialib_service::run() {
    zmq::pollitem_t items[] = {
        {management, 0, ZMQ_POLLIN, 0}
    };

    size_t items_size = sizeof(items) / sizeof(zmq::pollitem_t);

    try {
        send_status(frame::MedialibReady);

        while (!stopped) {
            loop(items, items_size);
        }
    } catch (std::exception& e) {
        LOG(logger, LOG_ERROR, "medialib: Error: %s", e.what());
        send_status(frame::MedialibPanic);
    }

    LOG(logger, LOG_INFO, "medialib: Closing");
    send_status(frame::MedialibExit);
}

void medialib_service::add_song(std::string path) {
    songs.push_back(path);
}

void medialib_service::loop(zmq::pollitem_t items[], size_t length) {
    zmq::poll(items, length, -1);

    if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(management, handlers);
    }
}
