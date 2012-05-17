#ifndef __INPUT_VORBIS_HPP__
#define __INPUT_VORBIS_HPP__

#include <vorbis/vorbisfile.h>

#include "input_base.hpp"

#include <string>
#include <memory>

class pcm_meta;
class pcm_format;
class pcm_packet;

void initialize_vorbis();

class input_vorbis : public input_base {
public:
  input_vorbis();
  ~input_vorbis();

  virtual void open(std::string path, pcm_format);
  virtual void close();
  virtual void seek(double pos);
  virtual pcm_position tell();
  virtual double length();

  virtual std::shared_ptr<pcm_packet> readsome();
private:
  int vf_bitstream;
  double vf_length;

  int bigendianp;
  int word;
  int sgned;

  pcm_format current_format;

  std::shared_ptr<vorbis_info> vf_info;
  std::shared_ptr<OggVorbis_File> vf_oggfile;
};

#endif /* __INPUT_VORBIS_HPP__ */
