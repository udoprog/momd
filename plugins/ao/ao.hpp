#ifndef __INPUT_AO_HPP__
#define __INPUT_AO_HPP__

#include "output_base.hpp"

#include <ao/ao.h>
#include <libconfig.h++>

#include <memory>

class pcm_packet;

class output_ao : public output_base {
public:
  output_ao();
  ~output_ao();

  virtual void setup(libconfig::Setting&);
  virtual void open();
  virtual bool is_open();
  virtual void close();
  virtual void write(std::shared_ptr<pcm_packet>);
  pcm_info info();

private:
  int driver;
  ao_device* device;
  ao_sample_format format;

  const char* server;
  const char* name;
  const char* stream;
};

#endif /* __INPUT_AO_HPP__ */
