#include "main_service.hpp"

#include "messages.hpp"
#include "log.hpp"

#include <functional>

#define MANAGEMENT(items) items[0]
#define MEDIALIB(items) items[1]
#define DECODER_DATA(items) items[2]
#define DECODER(items) items[3]
#define OUTPUT(items) items[4]

main_service::main_service(zmq::context_t& context)
   :  management(context, ZMQ_REP),
      decoder(context, ZMQ_PAIR),
      medialib(context, ZMQ_PAIR),
      output(context, ZMQ_PAIR),
      decoder_data(context, ZMQ_PULL),
      output_data(context, ZMQ_PUSH),
      logger(context, ZMQ_PUSH),
      handlers(),
      decoder_requested_frames(0),
      decoder_ready(false),
      playing(false),
      decoder_exit(false),
      output_exit(false),
      medialib_exit(false)
{
    using namespace std::placeholders;
    using namespace frame;
    using std::bind;

    management.bind("tcp://*:5555");

    decoder.bind("inproc://decoder");
    medialib.bind("inproc://medialib");
    output.bind("inproc://output");
    decoder_data.bind("inproc://decoder/data");
    output_data.bind("inproc://output/data");
    logger.connect("inproc://logger");

    handlers[Ping::TYPE] =
        bind(&main_service::ping, this, _1);

    handlers[Kill::TYPE] =
        bind(&main_service::kill, this, _1);

    handlers[Play::TYPE] =
        bind(&main_service::play, this, _1);

    handlers[Pause::TYPE] =
        bind(&main_service::pause, this, _1);

    handlers[RequestStatus::TYPE] =
        bind(&main_service::request_status, this, _1);

    handlers[DecoderError::TYPE] =
        bind(&main_service::decoder_error, this, _1);

    handlers[DecoderStatus::TYPE] =
        bind(&main_service::decoder_status, this, _1);

    handlers[OutputStatus::TYPE] =
        bind(&main_service::output_status, this, _1);

    handlers[MedialibNext::TYPE] =
        bind(&main_service::medialib_next, this, _1);

    handlers[MedialibStatus::TYPE] =
        bind(&main_service::medialib_status, this, _1);
}

main_service::~main_service() {
    medialib.close();
    management.close();
    decoder_data.close();
    decoder.close();
    output_data.close();
    output.close();
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
    send_frame(decoder, send_kill);
    send_frame(medialib, send_kill);
    send_frame(output, send_kill);
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
    status.decoder_requested_frames = decoder_requested_frames;

    send_frame(management, status);
}

void main_service::decoder_error(frame::frame_container& container)
{
    frame::DecoderError decoder_error;
    convert_frame(&container, &decoder_error);
}

void main_service::decoder_status(frame::frame_container& container)
{
    frame::DecoderStatus decoder_status;
    convert_frame(&container, &decoder_status);

    switch (decoder_status.status) {
        case frame::DecoderEndSong:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(EndSong)")
            decoder_ready = false;
            frame::MedialibRequestNext request_next;
            send_frame(medialib, request_next);
            break;
        case frame::DecoderFrameDropped:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(RequestFrameDropped)")
            --decoder_requested_frames;
            break;
        case frame::DecoderReady:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(Ready)")
            decoder_ready = true;
            break;
        case frame::DecoderExit:
            LOG(logger, LOG_DEBUG, "main: DecoderStatus(DecoderExit)")
            decoder_ready = false;
            decoder_exit = true;
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
            --decoder_requested_frames;
            break;
        case frame::OutputExit:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(OutputExit)")
            output_exit = true;
            break;
        default:
            LOG(logger, LOG_DEBUG, "main: OutputStatus(*)")
            break;
    }
}

void main_service::medialib_next(frame::frame_container& container)
{
    LOG(logger, LOG_DEBUG, "main: MedialibNext")

    frame::MedialibNext next;
    convert_frame(&container, &next);

    frame::DecoderNext next_song;
    next_song.path = next.path;
    send_frame(decoder, next_song);
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
        default:
            LOG(logger, LOG_DEBUG, "main: MedialibStatus(*)")
            break;
    }
}

void main_service::run()
{
    LOG(logger, LOG_DEBUG, "main: Initializing Decoder")
    frame::DecoderInitialize initialize;
    send_frame(decoder, initialize);

    zmq::pollitem_t items[] = {
        {management, 0, ZMQ_POLLIN, 0},
        {medialib, 0, ZMQ_POLLIN, 0},
        {decoder_data, 0, 0, 0},
        {decoder, 0, ZMQ_POLLIN, 0},
        {output, 0, ZMQ_POLLIN, 0}
    };

    while (true) {
        loop(items, sizeof(items) / sizeof(zmq::pollitem_t));

        bool done = medialib_exit
                &&  decoder_exit
                &&  output_exit;

        if (done) {
            break;
        }
    }
}

void main_service::loop(zmq::pollitem_t items[], size_t length) {
    DECODER_DATA(items).events = playing ? ZMQ_POLLIN : 0;
    MEDIALIB(items).events = !medialib_exit ? ZMQ_POLLIN : 0;
    OUTPUT(items).events = !output_exit ? ZMQ_POLLIN : 0;
    DECODER(items).events = !decoder_exit ? ZMQ_POLLIN : 0;

    /* fill up pcm buffer when necessary */
    while (    playing
            && decoder_ready
            && decoder_requested_frames < 1)
    {
        frame::DecoderRequestFrame request_frame;
        send_frame(decoder, request_frame);
        ++decoder_requested_frames;
    }

    zmq::poll(items, length, -1);

    /* handle management request-reply */
    if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(management, handlers);
    }

    if (MEDIALIB(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(medialib, handlers);
    }

    /* handle incoming decoder status updates */
    if (DECODER(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(decoder, handlers);
    }

    /* handle incoming output status updates */
    if (OUTPUT(items).revents & ZMQ_POLLIN) {
        frame::invoke_handler(output, handlers);
    }

    /* handle incoming decoder data */
    if (DECODER_DATA(items).revents & ZMQ_POLLIN) {
        zmq::message_t message;
        decoder_data.recv(&message);
        output_data.send(message);
    }
}
