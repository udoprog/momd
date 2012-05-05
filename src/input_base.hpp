#ifndef __INPUT_BASE__
#define __INPUT_BASE__

#include "pcm_info.hpp"

#include <memory>
#include <string>

class pcm_meta;
class pcm_packet;

class input_base {
public:
  virtual void open(std::string path) = 0;
  virtual void close() = 0;
  virtual void seek(double pos) = 0;
  virtual pcm_meta tell() = 0;
  virtual double length() = 0;

  virtual pcm_info info() = 0;

  /**
   * @returns Empty pcm_packet_ptr on end of stream.
   */
  virtual std::shared_ptr<pcm_packet> readsome() = 0;
  virtual ~input_base() {
  }
};

typedef std::shared_ptr<input_base> input_base_ptr;

#endif /*__INPUT_BASE__*/
