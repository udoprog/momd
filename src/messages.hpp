#ifndef __MESSAGES_HPP__
#define __MESSAGES_HPP__

#include "pcm_format.hpp"

#include <msgpack.hpp>
#include <zmq.hpp>
#include <vector>
#include <string>
#include <map>

#include <stdexcept>

#include <functional>

#define DECLARE_TYPE(Type) \
    const static uint8_t TYPE;

namespace frame {
    /**
     * The frame container.
     */
    class frame_container {
    private:
        uint8_t type;
        std::string payload;
    public:
        frame_container();
        frame_container(uint8_t type, std::string payload);

        uint8_t get_type();
        std::string get_payload();

        MSGPACK_DEFINE(type, payload)
    };

    #define DECLARE_TEMPLATE(Id, Class) \
        const uint8_t Class::TYPE = Id; \
        template void convert_frame<Class>(frame_container*, Class*); \
        template void send_frame<Class>(zmq::socket_t&, Class&);\
        template void send_router_frame<Class>(zmq::socket_t&, const char*, Class&);

    typedef std::function<void(frame_container&)> frame_handle_function;
    typedef std::map<uint8_t, frame_handle_function> handle_map;

    class invoke_exception : public std::runtime_error
    {
    public:
        explicit invoke_exception(const char*);
    };

    class frame_exception : public std::runtime_error
    {
    public:
        explicit frame_exception(const char*);
    };

    /**
     * Receive a frame container.
     */
    void receive_frame_container(zmq::socket_t&, frame_container*);

    /**
     * Convert a container into a frame.
     */
    template<typename T>
    void convert_frame(frame_container* container, T*);

    /**
     * Send a serialized reply on zmq socket.
     */
    template<typename T>
    void send_frame(zmq::socket_t&, T&);

    /**
     * Send a normal msgpack type.
     */
    template<typename T>
    void send_msgpack(zmq::socket_t& socket, T& object)
    {
        // serialize container
        msgpack::sbuffer object_buffer;
        msgpack::pack(object_buffer, object);

        // send container as a reply
        zmq::message_t reply (object_buffer.size());
        memcpy (reply.data(), object_buffer.data(), object_buffer.size());
        socket.send (reply);
    }

    /**
     * Send a normal msgpack type.
     */
    template<typename T>
    void receive_msgpack(zmq::socket_t& socket, T* object)
    {
        // receive message.
        zmq::message_t message;
        socket.recv(&message);

        msgpack::unpacked msg;

        try {
            msgpack::unpack(&msg, (const char*)message.data(), message.size()); 
        } catch (msgpack::unpack_error& error) {
            throw frame_exception("unpack message failed");
        }

        msgpack::object obj = msg.get();

        obj.convert(object);
    }

    /**
     * Send a serialized reply on zmq socket.
     */
    template<typename T>
    void send_router_frame(zmq::socket_t&, const char*, T&);

    /**
     * Convenience temlpate to invoke frame handler from a map of handlers.
     */
    void invoke_handler(zmq::socket_t& socket, handle_map& handlers);

    class Ping
    {
    public:
        int sequence;

        MSGPACK_DEFINE(sequence)
        DECLARE_TYPE(Ping)
    };

    class Kill
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(Kill)
    };

    class Panic
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(Panic)
    };

    class Error
    {
    public:
        std::string message;

        MSGPACK_DEFINE(message)
        DECLARE_TYPE(Error)
    };

    class Ok
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(Ok)
    };

    class Status
    {
    public:
        bool playing;
        bool decoder_ready;
        int output_pending_frames;

        MSGPACK_DEFINE(playing,
                decoder_ready,
                output_pending_frames)

        DECLARE_TYPE(Status)
    };

    class RequestStatus
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(RequestStatus)
    };

    class Pong
    {
    public:
        int sequence;

        MSGPACK_DEFINE(sequence)
        DECLARE_TYPE(Pong)
    };

    class Play
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(Play)
    };

    class Pause
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(Pause)
    };

    /**
     * Indicate next song to be played.
     *
     * path - Path to the next song to be played.
     */
    class DecoderNextSong
    {
    public:
        std::string path;
        pcm_format format;

        MSGPACK_DEFINE(path, format)
        DECLARE_TYPE(DecoderNextSong)
    };

    class DecoderRequestFrame
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(DecoderRequestFrame)
    };

    /**
     * Various decoder status that can be sent down the line.
     */
    enum DecoderStatusType {
        DecoderReady,
        DecoderExit,
        DecoderPanic,
        DecoderEndSong,
        DecoderFrameDropped,
        DecoderBusy
    };

    class DecoderStatus
    {
    public:
        int status;

        DecoderStatus();
        DecoderStatus(DecoderStatusType);

        MSGPACK_DEFINE(status)
        DECLARE_TYPE(DecoderStatus)
    };

    enum MedialibStatusType {
        MedialibReady,
        MedialibExit,
        MedialibPanic
    };

    class MedialibStatus
    {
    public:
        int status;

        MedialibStatus();
        MedialibStatus(MedialibStatusType);

        MSGPACK_DEFINE(status)
        DECLARE_TYPE(MedialibStatus)
    };

    /**
     * Various decoder status that can be sent down the line.
     */
    enum OutputStatusType {
        OutputReady,
        OutputExit,
        OutputPanic,
        OutputFrameReceived
    };

    class OutputStatus
    {
    public:
        int status;

        OutputStatus();
        OutputStatus(OutputStatusType);

        MSGPACK_DEFINE(status)
        DECLARE_TYPE(OutputStatus)
    };

    class OutputFormat
    {
    public:
        pcm_format format;

        MSGPACK_DEFINE(format)
        DECLARE_TYPE(OutputFormat)
    };

    class MedialibRequestNextSong
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(MedialibRequestNext)
    };

    class MedialibNextSong
    {
    public:
        std::string path;

        MSGPACK_DEFINE(path)
        DECLARE_TYPE(MedialibRequestNext)
    };
}

#endif /*__MESSAGES_HPP__*/
