#include "main_service.hpp"

#include "messages.hpp"
#include "log.hpp"

#include <functional>

#define MANAGEMENT(items) items[0]
#define DECODER_DATA(items) items[1]
#define SERVICES(items) items[2]

main_service::main_service(zmq::context_t& context)
   :  management(context, ZMQ_REP),
      services(context, ZMQ_ROUTER),
      decoder_data(context, ZMQ_PULL),
      output_data(context, ZMQ_PUSH),
      logger(context, ZMQ_PUSH),

      decoder_ready(false),
      decoder_exit(false),
      decoder_busy(false),
      decoder_pending_next_song(false),

      output_ready(false),
      output_exit(false),
      output_pending_frames(0),

      medialib_ready(false),
      medialib_exit(false),
      medialib_pending_next_song(false),

      playing(false),
      stopped(false),
      any_service_panic(false)
{
    using namespace std::placeholders;
    using namespace frame;
    using std::bind;

    management.bind("tcp://*:5555");
    services.bind("inproc://services");
    decoder_data.bind("inproc://decoder/data");
    output_data.bind("inproc://output/data");
    logger.connect("inproc://logger");

    management_handlers[Ping::TYPE] =
        bind(&main_service::ping, this, _1);

    management_handlers[Kill::TYPE] =
        bind(&main_service::kill, this, _1);

    management_handlers[Play::TYPE] =
        bind(&main_service::play, this, _1);

    management_handlers[Pause::TYPE] =
        bind(&main_service::pause, this, _1);

    management_handlers[RequestStatus::TYPE] =
        bind(&main_service::request_status, this, _1);

    service_handlers[DecoderStatus::TYPE] =
        bind(&main_service::decoder_status, this, _1);

    service_handlers[OutputStatus::TYPE] =
        bind(&main_service::output_status, this, _1);

    service_handlers[MedialibNextSong::TYPE] =
        bind(&main_service::medialib_next_song, this, _1);

    service_handlers[MedialibStatus::TYPE] =
        bind(&main_service::medialib_status, this, _1);
}

main_service::~main_service() {
    management.close();
    services.close();
    decoder_data.close();
    output_data.close();
    logger.close();
}

void main_service::ping(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: Ping")

    frame::Ping ping;
    convert_frame(&container, &ping);

    frame::Pong pong;
    pong.sequence = ping.sequence;

    send_frame(management, pong);
}

void main_service::kill(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: Kill")

    frame::Kill kill;
    convert_frame(&container, &kill);

    frame::Kill send_kill;
    send_router_frame(services, "decoder", send_kill);
    send_router_frame(services, "medialib", send_kill);
    send_router_frame(services, "output", send_kill);
}

void main_service::play(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: Play")

    frame::Play play;
    convert_frame(&container, &play);

    playing = true;

    frame::Ok ok;
    send_frame(management, ok);
}

void main_service::pause(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: Pause")

    frame::Pause pause;
    convert_frame(&container, &pause);

    playing = false;

    frame::Ok ok;
    send_frame(management, ok);
}

void main_service::request_status(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: RequestStatus")

    frame::RequestStatus request_status;
    convert_frame(&container, &request_status);

    frame::Status status;
    status.playing = playing;
    status.decoder_ready = decoder_ready;
    status.output_pending_frames = output_pending_frames;

    send_frame(management, status);
}

void main_service::decoder_status(frame::frame_container& container)
{
    frame::DecoderStatus decoder_status;
    convert_frame(&container, &decoder_status);

    switch (decoder_status.status) {
        case frame::DecoderReady:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderReady)")
            break;
        case frame::DecoderExit:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderExit)")
            decoder_ready = false;
            decoder_exit = true;
            break;
        case frame::DecoderBusy:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderBusy)")
            decoder_pending_next_song = false;
            decoder_busy = true;
            break;
        case frame::DecoderEndSong:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderEndSong)")
            decoder_pending_next_song = false;
            decoder_busy = false;
            break;
        case frame::DecoderFrameDropped:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderFrameDropped)")
            break;
        case frame::DecoderPanic:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderPanic)")
            any_service_panic = true;
            break;
        default:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(*)")
            break;
    }
}

void main_service::output_status(frame::frame_container& container)
{
    frame::OutputStatus status;
    convert_frame(&container, &status);

    switch (status.status) {
        case frame::OutputFrameReceived:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(OutputFrameReceived)")
            --output_pending_frames;
            break;
        case frame::OutputReady:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(OutputReady)")
            output_ready = true;
            break;
        case frame::OutputExit:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(OutputExit)")
            output_ready = false;
            output_exit = true;
            break;
        case frame::OutputPanic:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(OutputPanic)")
            any_service_panic = true;
            break;
        default:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(*)")
            break;
    }
}

void main_service::medialib_next_song(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: MedialibNextSong")

    medialib_pending_next_song = false;

    frame::MedialibNextSong next;
    convert_frame(&container, &next);

    frame::DecoderNextSong next_song;
    next_song.path = next.path;
    frame::send_router_frame(services, "decoder", next_song);

    decoder_pending_next_song = true;
}

void main_service::medialib_status(frame::frame_container& container)
{
    frame::MedialibStatus status;
    convert_frame(&container, &status);

    switch (status.status) {
        case frame::MedialibExit:
            LOG(logger, LOG_DEBUG, "main: MedialibStatus(MedialibExit)")
            medialib_exit = true;
            break;
        case frame::MedialibReady:
            LOG(logger, LOG_DEBUG, "main: MedialibStatus(MedialibReady)")
            medialib_ready = true;
            break;
        case frame::MedialibPanic:
            LOG(logger, LOG_DEBUG, "main: MedialibStatus(MedialibPanic)")
            any_service_panic = true;
            break;
        default:
            LOG(logger, LOG_DEBUG, "main: MedialibStatus(*)")
            break;
    }
}

void main_service::handle_panic()
{
    LOG(logger, LOG_ERROR, "main: A service panic has been detected")
    
    frame::Kill send_kill;

    if (!decoder_exit) {
        send_router_frame(services, "decoder", send_kill);
    }

    if (!medialib_exit) {
        send_router_frame(services, "medialib", send_kill);
    }

    if (!output_exit) {
        send_router_frame(services, "output", send_kill);
    }
}

void main_service::run()
{
    zmq::pollitem_t items[] = {
        {management, 0, ZMQ_POLLIN, 0},
        {decoder_data, 0, ZMQ_POLLIN, 0},
        {services, 0, ZMQ_POLLIN, 0}
    };

    size_t items_size = sizeof(items) / sizeof(zmq::pollitem_t);

    while (true) {
        loop(items, items_size);

        bool done = medialib_exit
                &&  decoder_exit
                &&  output_exit;

        if (any_service_panic) {
            handle_panic();
            break;
        }

        if (done) {
            break;
        }
    }
}

std::string main_service::receive_identity()
{
    zmq::message_t identity;
    services.recv(&identity);
    return std::string((const char*)identity.data(), identity.size());
}

void main_service::handle_playing()
{
    /*
     decoder_busy - Decoder is actively working.
     medialib_pending_next_song - Next song request is pending for medialib.
     decoder_pending_next-song - Next song is already pending for decoder.

     Routine should only request new songs from medialib if one is not being
     actively decoded.
     */
    if (!decoder_busy
     && !medialib_pending_next_song
     && !decoder_pending_next_song
    )
    {
        frame::MedialibRequestNextSong request_next;
        send_router_frame(services, "medialib", request_next);
        medialib_pending_next_song = true;
    }

    /*
     A new frame should only be requested if decoder is actively working and
     output is accepting frames.
     */
    if (decoder_busy
     && output_ready
     && output_pending_frames < 5
    )
    {
        frame::DecoderRequestFrame request_frame;
        send_router_frame(services, "decoder", request_frame);
        ++output_pending_frames;
    }
}

void main_service::loop(zmq::pollitem_t items[], size_t length) {
    DECODER_DATA(items).events = playing ? ZMQ_POLLIN : 0;

    if (playing) {
        handle_playing();
    }

    zmq::poll(items, length, -1);

    if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(management, management_handlers);
    }

    if (SERVICES(items).revents & ZMQ_POLLIN) {
        std::string identity = receive_identity();
        LOG(logger, LOG_INFO, "main: message from '%s'", identity.c_str())
        frame::invoke_handler(services, service_handlers);
    }

    /* ferry incoming requests from decoder to output */
    if (DECODER_DATA(items).revents & ZMQ_POLLIN) {
        zmq::message_t message;
        LOG(logger, LOG_DEBUG, "main: decoder_data -> output_data")
        decoder_data.recv(&message);
        output_data.send(message);
    }
}
