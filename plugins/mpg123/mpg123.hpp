#ifndef __INPUT_MPG123_HPP__
#define __INPUT_MPG123_HPP__

#include "input.hpp"

#include <mpg123.h>
#include <string>

class pcm_meta;
class pcm_info;
class pcm_packet;

class input_base;

void initialize_mpg123();

class input_mpg123 : public input_base {
public:
  input_mpg123();
  ~input_mpg123();

  virtual void open(std::string path);
  virtual void close();
  virtual void seek(double pos);
  virtual pcm_meta tell();
  virtual double length();

  virtual pcm_info info();
  virtual pcm_packet_ptr readsome();
private:
  mpg123_handle* handle;

  int _channels;
  long int _rate;
  int _encoding;
  double _length;
};

#endif /* __INPUT_MPG123_HPP__ */
