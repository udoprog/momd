#include "decoder_event_loop.hpp"

#include "input.hpp"
#include "io.hpp"

#include "messages.hpp"

#include <stdexcept>

class decoder_state
{
public:
    input_base_ptr input;
    zmq::socket_t& decoder_status;
    zmq::socket_t& decoder_data;

    decoder_state(
            zmq::socket_t& decoder_status,
            zmq::socket_t& decoder_data)
        : decoder_status(decoder_status),
          decoder_data(decoder_data)
    {
    }
};

void send_decoder_status(zmq::socket_t& socket, DecoderStatusType status_type)
{
    DecoderStatus status(status_type);
    send_frame(socket, status);
}

void handle_decoder_next_song(void* user_data, frame_container& container)
{
    std::cerr << "decoder: DecoderNextSong" << std::endl;

    decoder_state* state = (decoder_state*)user_data;

    DecoderNextSong next_song;
    convert_frame(&container, &next_song);

    try {
        state->input = open_path(next_song.path);
    } catch (std::exception& e) {
        send_decoder_status(state->decoder_status, EndSong);
        return;
    }

    send_decoder_status(state->decoder_status, Ready);
}

void handle_decoder_request_frame(void* user_data, frame_container& container)
{
    std::cerr << "decoder: DecoderRequestFrame" << std::endl;

    decoder_state* state = (decoder_state*)user_data;

    if (!state->input) {
        send_decoder_status(state->decoder_status, RequestFrameDropped);
        return;
    }

    pcm_packet_ptr pcm = state->input->readsome();

    if (!pcm) {
        // set input to false and request another song.
        state->input = input_base_ptr();
        send_decoder_status(state->decoder_status, EndSong);
        return;
    }

    zmq::message_t message(pcm->size());
    ::memcpy(message.data(), pcm->data(), pcm->size());
    state->decoder_data.send(message);
}

void handle_decoder_initialize(void* user_data, frame_container& container)
{
    std::cerr << "decoder: DecoderInitialize" << std::endl;

    decoder_state* state = (decoder_state*)user_data;
    send_decoder_status(state->decoder_status, EndSong);
}

void setup_decoder_handlers(handle_map& decoder_handlers) {
    decoder_handlers[DecoderNextSong::TYPE] = handle_decoder_next_song;
    decoder_handlers[DecoderRequestFrame::TYPE] = handle_decoder_request_frame;
    decoder_handlers[DecoderInitialize::TYPE] = handle_decoder_initialize;
}

#define DECODER_MANAGEMENT(items) items[0]

void decoder_event_loop(zmq::context_t* context)
{
    handle_map decoder_handlers;
    setup_decoder_handlers(decoder_handlers);

    zmq::socket_t decoder_data(*context, ZMQ_PUSH);

    {
        decoder_data.connect("inproc://decoder/data");
        uint64_t hwm = 10;
        decoder_data.setsockopt(ZMQ_HWM, &hwm, sizeof(hwm));
    }

    zmq::socket_t decoder_management(*context, ZMQ_PULL);
    decoder_management.connect("inproc://decoder/management");

    zmq::socket_t decoder_status(*context, ZMQ_PUSH);
    decoder_status.connect("inproc://decoder/status");

    zmq::pollitem_t items[] = {
        {decoder_management, 0, ZMQ_POLLIN, 0}
    };

    decoder_state state(decoder_status, decoder_data);

    while (true) {
        zmq::poll(items, 1, -1);

        if (DECODER_MANAGEMENT(items).revents & ZMQ_POLLIN) {
            frame_container container;
            receive_frame_container(decoder_management, &container);
            invoke_frame_handler(&state, decoder_handlers, container);
        }
    }
}
