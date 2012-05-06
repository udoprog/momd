#include "decoder_service.hpp"

#include "input.hpp"
#include "messages.hpp"
#include "log.hpp"
#include "pcm_packet.hpp"
#include "input_base.hpp"

#include <functional>

#define MANAGEMENT(items) items[0]

decoder_service::decoder_service(zmq::context_t* context)
    : management(*context, ZMQ_PAIR),
      decoder_data(*context, ZMQ_PUSH),
      logger(*context, ZMQ_PUSH),
      handlers(),
      stopped(false)
{
    using namespace std::placeholders;
    using namespace frame;
    using std::bind;

    decoder_data.connect("inproc://decoder/data");
    management.connect("inproc://decoder");
    logger.connect("inproc://logger");

    handlers[Kill::TYPE] =
        bind(&decoder_service::kill, this, _1);

    handlers[DecoderNext::TYPE] =
        bind(&decoder_service::next_song, this, _1);

    handlers[DecoderRequestFrame::TYPE] =
        bind(&decoder_service::decoder_request_frame, this, _1);

    handlers[DecoderInitialize::TYPE] =
        bind(&decoder_service::decoder_initialize, this, _1);
}

decoder_service::~decoder_service() {
    decoder_data.close();
    management.close();
    logger.close();
}

void decoder_service::kill(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "decoder: Kill")
    stopped = true;
}

void decoder_service::next_song(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "decoder: DecoderNext");

    frame::DecoderNext next_song;
    convert_frame(&container, &next_song);

    try {
        input = open_path(next_song.path);
    } catch (std::exception& e) {
        send_status(frame::DecoderEndSong);
        return;
    }

    send_status(frame::DecoderReady);
}

void decoder_service::decoder_request_frame(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "decoder: DecoderRequestFrame");

    if (!input) {
        send_status(frame::DecoderFrameDropped);
        return;
    }

    pcm_packet::ptr pcm = input->readsome();

    if (!pcm) {
        // set input to false and request another song.
        input = input_base_ptr();
        send_status(frame::DecoderEndSong);
        return;
    }

    zmq::message_t message(pcm->size());
    ::memcpy(message.data(), pcm->data(), pcm->size());
    decoder_data.send(message);
}

void decoder_service::decoder_initialize(frame::frame_container& container) {
    LOG(logger, LOG_DEBUG, "decoder: DecoderInitialize");

    send_status(frame::DecoderEndSong);
}

void decoder_service::send_status(frame::DecoderStatusType status_type) {
    frame::DecoderStatus status(status_type);
    send_frame(management, status);
}

void decoder_service::run() {
    zmq::pollitem_t items[] = {
        {management, 0, ZMQ_POLLIN, 0}
    };

    try {
        while (!stopped) {
            loop(items, sizeof(items) / sizeof(zmq::pollitem_t));
        }

        LOG(logger, LOG_INFO, "decoder: Closing");
    } catch (std::exception& e) {
        LOG(logger, LOG_ERROR, "decoder: Error: %s", e.what());
    }

    send_status(frame::DecoderExit);
}

void decoder_service::loop(zmq::pollitem_t items[], size_t length) {
    zmq::poll(items, 1, -1);

    if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(management, handlers);
    }
}
