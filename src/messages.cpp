#include <algorithm>
#include <zmq.hpp>
#include <msgpack.hpp>

#include "messages.hpp"

#include <stdexcept>

frame_container::frame_container()
    : type(0x0), payload()
{
}

frame_container::frame_container(uint8_t type, std::string payload)
    : type(type), payload(payload)
{

}

uint8_t frame_container::get_type()
{
    return type;
}

std::string frame_container::get_payload()
{
    return payload;
}

void invoke_frame_handler(
    void* user_data,
    handle_map& handlers,
    frame_container& container)
{
    handle_map::iterator result = handlers.find(container.get_type());

    if (result == handlers.end()) {
        throw std::exception();
    }

    frame_handle_function function = result->second;

    function(user_data, container);
}

void receive_frame_container(zmq::socket_t& socket, frame_container* target)
{
    // receive container from client
    zmq::message_t container;
    socket.recv(&container);

    msgpack::unpacked msg;
    msgpack::unpack(&msg, (const char*)container.data(), container.size()); 
    msgpack::object obj = msg.get();

    obj.convert(target);
}

template<typename T>
void convert_frame(frame_container* container, T* target)
{
    std::string payload = container->get_payload();

    msgpack::unpacked unpacked;

    msgpack::unpack(&unpacked, payload.c_str(), payload.size());
    msgpack::object obj = unpacked.get();

    obj.convert(target);
}

template<typename T>
void send_frame(zmq::socket_t& socket, T& frame)
{
    // serialize frame
    msgpack::sbuffer frame_buffer;
    msgpack::pack(frame_buffer, frame);

    std::string frame_string(frame_buffer.data(), frame_buffer.size());

    // build container
    frame_container container(T::TYPE, frame_string);

    // serialize container
    msgpack::sbuffer container_buffer;
    msgpack::pack(container_buffer, container);

    // send container as a reply
    zmq::message_t reply (container_buffer.size());
    memcpy (reply.data(), container_buffer.data(), container_buffer.size());
    socket.send (reply);
}

#define DECLARE_TEMPLATES(T, C) \
    const uint8_t C::TYPE = T; \
    template void convert_frame<C>(frame_container*, C*); \
    template void send_frame<C>(zmq::socket_t&, C&);

DECLARE_TEMPLATES(0x01, Ping)
DECLARE_TEMPLATES(0x02, Pong)
DECLARE_TEMPLATES(0x04, RequestStatus)
DECLARE_TEMPLATES(0x05, Status)
DECLARE_TEMPLATES(0x10, Play)
DECLARE_TEMPLATES(0x11, Pause)
DECLARE_TEMPLATES(0x80, Error)
DECLARE_TEMPLATES(0x90, Ok)

/**
 * Decode frames.
 */
DECLARE_TEMPLATES(0x21, DecoderNextSong)
DECLARE_TEMPLATES(0x22, DecoderInitialize)
DECLARE_TEMPLATES(0x40, DecoderError)
DECLARE_TEMPLATES(0x41, DecoderStatus)

DecoderStatus::DecoderStatus()
    : status(0)
{
}

DecoderStatus::DecoderStatus(DecoderStatusType status)
    : status(status)
{
}

DECLARE_TEMPLATES(0x42, DecoderRequestFrame)

DECLARE_TEMPLATES(0x51, OutputStatus)

OutputStatus::OutputStatus()
    : status(0)
{
}

OutputStatus::OutputStatus(OutputStatusType status)
    : status(status)
{
}
