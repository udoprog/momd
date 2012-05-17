#include <algorithm>
#include <zmq.hpp>
#include <msgpack.hpp>

#include "messages.hpp"

#include <stdexcept>
#include <cstring>

namespace frame {
    invoke_exception::invoke_exception(const char* message)
        : std::runtime_error(message)
    {
    }

    frame_exception::frame_exception(const char* message)
        : std::runtime_error(message)
    {
    }

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

    void invoke_handler(zmq::socket_t& socket, handle_map& handlers)
    {
        frame_container container;

        receive_frame_container(socket, &container);

        handle_map::iterator result = handlers.find(container.get_type());

        if (result == handlers.end()) {
            throw invoke_exception("no handler found");
        }

        frame_handle_function function = result->second;

        function(container);
    }

    void receive_frame_container(zmq::socket_t& socket, frame_container* target)
    {
        // receive container from client
        zmq::message_t container;
        socket.recv(&container);

        msgpack::unpacked msg;

        try {
            msgpack::unpack(&msg, (const char*)container.data(), container.size()); 
        } catch (msgpack::unpack_error& error) {
            throw frame_exception("unpack container failed");
        }

        msgpack::object obj = msg.get();

        obj.convert(target);
    }

    template<typename T>
    void convert_frame(frame_container* container, T* target)
    {
        std::string payload = container->get_payload();

        msgpack::unpacked unpacked;

        try {
            msgpack::unpack(&unpacked, payload.c_str(), payload.size());
        } catch (msgpack::unpack_error& error) {
            throw frame_exception("convert frame failed");
        }

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

    template<typename T>
    void send_router_frame(zmq::socket_t& socket, const char* target, T& frame)
    {
        size_t length = ::strlen(target);
        zmq::message_t identity(length);
        ::memcpy(identity.data(), target, length);
        socket.send(identity, ZMQ_SNDMORE);
        send_frame(socket, frame);
    }


    DECLARE_TEMPLATE(0x01, Ping)
    DECLARE_TEMPLATE(0x02, Pong)
    DECLARE_TEMPLATE(0x04, RequestStatus)
    DECLARE_TEMPLATE(0x05, Status)
    DECLARE_TEMPLATE(0x10, Play)
    DECLARE_TEMPLATE(0x11, Pause)
    DECLARE_TEMPLATE(0x80, Error)
    DECLARE_TEMPLATE(0x90, Ok)
    DECLARE_TEMPLATE(0x09, Kill)
    DECLARE_TEMPLATE(0x90, Panic)

    /**
     * Decode frames.
     */
    DECLARE_TEMPLATE(0x21, DecoderNextSong)
    DECLARE_TEMPLATE(0x41, DecoderStatus)
    DECLARE_TEMPLATE(0x42, DecoderRequestFrame)

    DecoderStatus::DecoderStatus()
        : status(0)
    {
    }

    DecoderStatus::DecoderStatus(DecoderStatusType status)
        : status(status)
    {
    }

    DECLARE_TEMPLATE(0x52, OutputFormat)
    DECLARE_TEMPLATE(0x51, OutputStatus)

    OutputStatus::OutputStatus()
        : status(0)
    {
    }

    OutputStatus::OutputStatus(OutputStatusType status)
        : status(status)
    {
    }

    DECLARE_TEMPLATE(0x60, MedialibRequestNextSong)
    DECLARE_TEMPLATE(0x61, MedialibNextSong)

    DECLARE_TEMPLATE(0x71, MedialibStatus)

    MedialibStatus::MedialibStatus()
        : status(0)
    {
    }

    MedialibStatus::MedialibStatus(MedialibStatusType status)
        : status(status)
    {
    }
}
