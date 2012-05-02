#ifndef __INPUT_VORBIS_HPP__
#define __INPUT_VORBIS_HPP__

#include <vorbis/vorbisfile.h>

#include "input.hpp"

#include <string>
#include <memory>

class pcm_meta;
class pcm_info;
class pcm_packet;

void initialize_vorbis();

class input_vorbis : public input_base {
public:
  typedef std::shared_ptr<pcm_packet> pcm_packet_ptr;

  input_vorbis();
  ~input_vorbis();

  virtual void open(std::string path);
  virtual void close();
  virtual void seek(double pos);
  virtual pcm_meta tell();
  virtual double length();

  virtual pcm_info info();
  virtual pcm_packet_ptr readsome();
private:
  int _bitstream;
  int _endian;
  double _length;

  std::shared_ptr<vorbis_info> _info;
  std::shared_ptr<OggVorbis_File> _oggfile;
};

#endif /* __INPUT_VORBIS_HPP__ */
