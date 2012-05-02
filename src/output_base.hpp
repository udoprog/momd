#ifndef __OUTPUT_BASE__
#define __OUTPUT_BASE__

#include <libconfig.h++>
#include <memory>

class pcm_packet;
class pcm_info;

class output_base {
public:
  typedef std::shared_ptr<pcm_packet> pcm_packet_ptr;

  virtual void setup(libconfig::Setting&) = 0;
  virtual void open() = 0;
  virtual bool is_open() = 0;
  virtual void close() = 0;
  virtual void write(pcm_packet_ptr) = 0;
  virtual pcm_info info() = 0;

  void set_name(std::string);
  std::string name();
private:
  std::string _name;
};

typedef std::shared_ptr<output_base> output_base_ptr;

#endif /* __OUTPUT_BASE__ */
