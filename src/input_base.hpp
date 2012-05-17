#ifndef __INPUT_BASE__
#define __INPUT_BASE__

#include "pcm_format.hpp"

#include <memory>
#include <string>

class pcm_format;
class pcm_info;
class pcm_packet;
class pcm_position;

class input_base {
public:
    typedef std::shared_ptr<input_base> ptr;

     virtual void open(std::string path, pcm_format format) = 0;
     virtual void close() = 0;
     virtual void seek(double pos) = 0;
     virtual pcm_position tell() = 0;
     virtual double length() = 0;
    
     /**
      * @returns Empty pcm_packet_ptr on end of stream.
      */
     virtual std::shared_ptr<pcm_packet> readsome() = 0;
     virtual ~input_base() { }
};

#endif /*__INPUT_BASE__*/
