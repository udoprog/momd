#include "main_event_loop.hpp"
#include "decoder_event_loop.hpp"
#include "output_event_loop.hpp"

#include "messages.hpp"

#include "output/pulse.hpp"

#include <thread>

class player_state
{
public:
    zmq::socket_t& management;
    zmq::socket_t& decoder_management;
    int decoder_requested_frames;
    bool decoder_ready;
    bool playing;

    player_state(
            zmq::socket_t& management,
            zmq::socket_t& decoder_management)
        : management(management),
          decoder_management(decoder_management),
          decoder_requested_frames(0),
          decoder_ready(false),
          playing(false)
    {
    }
};

void recv_and_handle_management(
        player_state& state,
        handle_map& management_handlers)
{
    // receive frame container to figure out the type of the frame.
    frame_container container;

    try {
        receive_frame_container(state.management, &container);
    } catch (std::exception& e) {
        Error error;
        error.message = "internal server error (container)";
        send_frame(state.management, error);
        return;
    }

    void* user_data = &state;

    try {
        invoke_frame_handler(user_data, management_handlers, container);
    } catch (std::exception &e) {
        Error error;
        error.message = "internal server error (invoke)";
        send_frame(state.management, error);
    }
}

void handle_ping(void* user_data, frame_container& container)
{
    std::cerr << "main: Ping" << std::endl;

    player_state* state = (player_state*)user_data;

    Ping ping;
    convert_frame(&container, &ping);

    Pong pong;
    pong.sequence = ping.sequence;

    send_frame(state->management, pong);
}

void handle_play(void* user_data, frame_container& container)
{
    std::cerr << "main: Play" << std::endl;

    player_state* state = (player_state*)user_data;

    Play play;
    convert_frame(&container, &play);

    state->playing = true;

    Ok ok;
    send_frame(state->management, ok);
}

void handle_pause(void* user_data, frame_container& container)
{
    std::cerr << "main: Pause" << std::endl;

    player_state* state = (player_state*)user_data;

    Pause pause;
    convert_frame(&container, &pause);

    state->playing = false;

    Ok ok;
    send_frame(state->management, ok);
}

void handle_request_status(void* user_data, frame_container& container)
{
    std::cerr << "main: RequestStatus" << std::endl;

    player_state* state = (player_state*)user_data;

    RequestStatus request_status;
    convert_frame(&container, &request_status);

    Status status;
    status.playing = state->playing;
    status.decoder_ready = state->decoder_ready;
    status.decoder_requested_frames = state->decoder_requested_frames;

    send_frame(state->management, status);
}


void send_decoder_next_song(zmq::socket_t& socket)
{
    DecoderNextSong next_song;
    next_song.path = "test.mp3";
    send_frame(socket, next_song);
}

void setup_management_handlers(handle_map& management_handlers) {
    management_handlers[Ping::TYPE] = handle_ping;
    management_handlers[Play::TYPE] = handle_play;
    management_handlers[Pause::TYPE] = handle_pause;
    management_handlers[RequestStatus::TYPE] = handle_request_status;
}

void handle_decoder_error(void*, frame_container& container)
{
    DecoderError decoder_error;
    convert_frame(&container, &decoder_error);
}

void handle_decoder_status(void* user_data, frame_container& container)
{
    player_state* state = (player_state*)user_data;

    DecoderStatus decoder_status;
    convert_frame(&container, &decoder_status);

    switch (decoder_status.status) {
        case EndSong:
            std::cerr << "main: DecoderStatus(EndSong)" << std::endl;

            send_decoder_next_song(state->decoder_management);
            state->decoder_ready = false;
            break;
        case RequestFrameDropped:
            std::cerr << "main: DecoderStatus(RequestFrameDropped)" << std::endl;

            --state->decoder_requested_frames;
            break;
        case Ready:
            std::cerr << "main: DecoderStatus(Ready)" << std::endl;

            state->decoder_ready = true;
            break;
        default:
            std::cerr << "main: DecoderStatus(*)" << std::endl;
            break;
    }
}

void setup_decoder_status_handlers(handle_map& decoder_status_handlers) {
    decoder_status_handlers[DecoderError::TYPE] = handle_decoder_error;
    decoder_status_handlers[DecoderStatus::TYPE] = handle_decoder_status;
}

void handle_output_status(void* user_data, frame_container& container)
{
    player_state* state = (player_state*)user_data;

    OutputStatus output_status;
    convert_frame(&container, &output_status);

    switch (output_status.status) {
        case FrameReceived:
            std::cerr << "main: OutputStatus(FrameReceived)" << std::endl;
            --state->decoder_requested_frames;
            break;
        default:
            std::cerr << "main: OutputStatus(*)" << std::endl;
            break;
    }
}

void setup_output_status_handlers(handle_map& output_status_handlers) {
    output_status_handlers[OutputStatus::TYPE] = handle_output_status;
}

#define MANAGEMENT(items) items[0]
#define DECODER_DATA(items) items[1]
#define DECODER_STATUS(items) items[2]
#define OUTPUT_STATUS(items) items[3]

void main_event_loop(libconfig::Config& config, zmq::context_t& context)
{
    handle_map management_handlers;
    setup_management_handlers(management_handlers);

    handle_map decoder_status_handlers;
    setup_decoder_status_handlers(decoder_status_handlers);

    handle_map output_status_handlers;
    setup_output_status_handlers(output_status_handlers);

    zmq::socket_t management(context, ZMQ_REP);
    management.bind("tcp://*:5555");

    zmq::socket_t decoder_data(context, ZMQ_PULL);
    decoder_data.bind("inproc://decoder/data");

    zmq::socket_t decoder_status(context, ZMQ_PULL);
    decoder_status.bind("inproc://decoder/status");

    zmq::socket_t decoder_management(context, ZMQ_PUSH);
    decoder_management.bind("inproc://decoder/management");

    zmq::socket_t output_data(context, ZMQ_PUSH);
    output_data.bind("inproc://output/data");

    zmq::socket_t output_status(context, ZMQ_PULL);
    output_status.bind("inproc://output/status");

    std::thread decoder_thread(decoder_event_loop, &context);
    std::thread output_thread(output_event_loop, &context, &config);

    zmq::pollitem_t items[] = {
        {management, 0, ZMQ_POLLIN, 0},
        {decoder_data, 0, ZMQ_POLLIN, 0},
        {decoder_status, 0, ZMQ_POLLIN, 0},
        {output_status, 0, ZMQ_POLLIN, 0}
    };

    std::cout << "main: intializing decoder" << std::endl;

    {
        DecoderInitialize initialize;
        send_frame(decoder_management, initialize);
    }

    player_state state(management, decoder_management);

    while (true) {
        while (    state.playing
                && state.decoder_ready
                && state.decoder_requested_frames < 10)
        {
            DecoderRequestFrame request_frame;
            send_frame(decoder_management, request_frame);
            ++state.decoder_requested_frames;
        }

        zmq::poll(items, 4, -1);

        if (MANAGEMENT(items).revents & ZMQ_POLLIN) {
            recv_and_handle_management(state, management_handlers);
        }

        if (DECODER_DATA(items).revents & ZMQ_POLLIN) {
            zmq::message_t message;
            decoder_data.recv(&message);

            std::cerr << "main: decoder -> output "
                << message.size() << " bytes" << std::endl;

            zmq::message_t output_message(message.size());
            ::memcpy(output_message.data(), message.data(), message.size());
            output_data.send(output_message);
        }

        if (DECODER_STATUS(items).revents & ZMQ_POLLIN) {
            frame_container container;
            receive_frame_container(decoder_status, &container);
            invoke_frame_handler(&state, decoder_status_handlers, container);
        }

        if (OUTPUT_STATUS(items).revents & ZMQ_POLLIN) {
            frame_container container;
            receive_frame_container(output_status, &container);
            invoke_frame_handler(&state, output_status_handlers, container);
        }
    }
}
