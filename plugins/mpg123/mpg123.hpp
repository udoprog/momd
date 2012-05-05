#ifndef __INPUT_MPG123_HPP__
#define __INPUT_MPG123_HPP__

#include "input_base.hpp"

#include <string>
#include <unc/unc.hpp>
#include <mpg123.h>

class pcm_meta;
class pcm_info;
class pcm_packet;
class input_base;

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
  virtual std::shared_ptr<pcm_packet> readsome();
private:
  void read_metadata();

  mpg123_handle* handle;

  int _channels;
  long int _rate;
  int _encoding;
  double _length;

  unc::ustring title;
  unc::ustring artist;
  unc::ustring album;
  unc::ustring year;
};

void initialize_mpg123();

#endif /* __INPUT_MPG123_HPP__ */
