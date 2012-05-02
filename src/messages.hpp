#ifndef __MESSAGES_HPP__
#define __MESSAGES_HPP__

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
        template void send_frame<Class>(zmq::socket_t&, Class&);

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
        int decoder_requested_frames;

        MSGPACK_DEFINE(playing,
                decoder_ready,
                decoder_requested_frames)

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
    class DecoderNext
    {
    public:
        std::string path;

        MSGPACK_DEFINE(path)
        DECLARE_TYPE(DecoderNext)
    };

    class DecoderRequestFrame
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(DecoderRequestFrame)
    };

    class DecoderInitialize
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(DecoderInitialize)
    };

    class DecoderError
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(DecoderError)
    };

    /**
     * Various decoder status that can be sent down the line.
     */
    enum DecoderStatusType {
        DecoderEndSong,
        DecoderReady,
        DecoderFrameDropped,
        DecoderExit
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
        MedialibExit
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
        OutputFrameReceived,
        OutputExit
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

    class MedialibRequestNext
    {
    public:
        MSGPACK_DEFINE()
        DECLARE_TYPE(MedialibRequestNext)
    };

    class MedialibNext
    {
    public:
        std::string path;
        MSGPACK_DEFINE(path)
        DECLARE_TYPE(MedialibRequestNext)
    };
}

#endif /*__MESSAGES_HPP__*/
