#ifndef __INPUT_PULSE_HPP__
#define __INPUT_PULSE_HPP__

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#include <libconfig.h++>

#include "output.hpp"

namespace fs = boost::filesystem;

class output_pulse : public output_base {
public:
  output_pulse();
  ~output_pulse();

  virtual void setup(libconfig::Setting&);
  virtual void open();
  virtual bool is_open();
  virtual void close();
  virtual void write(pcm_packet_ptr);
  pcm_info info();

private:
  boost::shared_ptr<pa_sample_spec> _spec;

  pa_simple* simple;

  const char* server;
  const char* name;
  const char* stream;
};

#endif /* __INPUT_PULSE_HPP__ */
