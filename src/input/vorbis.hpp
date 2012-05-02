#ifndef __INPUT_VORBIS_HPP__
#define __INPUT_VORBIS_HPP__

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <vorbis/vorbisfile.h>

#include "input.hpp"

class pcm_meta;
class pcm_info;
class pcm_packet;

namespace fs = boost::filesystem;

void initialize_vorbis();

class input_vorbis : public input_base {
public:
  typedef boost::shared_ptr<pcm_packet> pcm_packet_ptr;

  input_vorbis();
  ~input_vorbis();

  virtual void open(fs::path path);
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

  boost::scoped_ptr<vorbis_info> _info;
  boost::scoped_ptr<OggVorbis_File> _oggfile;
};

#endif /* __INPUT_VORBIS_HPP__ */
