#include "vorbis.hpp"

#include "plugin.hpp"
#include "input_error.hpp"
#include "pcm_meta.hpp"
#include "pcm_packet.hpp"

#include <vorbis/vorbisfile.h>

input_base* vorbis__new();
void vorbis__initialize();
bool vorbis__check(const char* path, const char* ext);

input_plugin_spec vorbis_spec = {
    &vorbis__initialize,
    &vorbis__check,
    &vorbis__new
};

DECLARE_INPUT_PLUGIN("vorbis", vorbis_spec)

input_base* vorbis__new() {
    return new input_vorbis();
}

void vorbis__initialize() {
}

std::string ogg_ext(".ogg");

bool vorbis__check(const char* path, const char* ext) {
    std::string string_path(path);
    std::string string_ext(ext);

    if (ogg_ext.compare(string_ext) == 0) {
        return true;
    }

    return false;
}

input_vorbis::input_vorbis()
  : _bitstream(0), _endian(0), _length(0.0)
{
    _oggfile.reset(new OggVorbis_File);
    _info.reset(new vorbis_info);
}

input_vorbis::~input_vorbis()
{
}

void input_vorbis::open(std::string path)
{
  FILE* fd = fopen(path.c_str(), "rb");

  if (fd == NULL) {
    throw input_error("failed to open file");
  }

  ov_open(fd, _oggfile.get(), NULL, 0);
  _info.reset(ov_info(_oggfile.get(), -1));
  this->_length = ov_time_total(_oggfile.get(), -1);
}

void input_vorbis::close()
{

}

void input_vorbis::seek(double pos)
{
  ov_time_seek(_oggfile.get(), pos);
}

pcm_meta input_vorbis::tell()
{
  double now = ov_time_tell(_oggfile.get());

  pcm_meta meta;
  meta.current = now;
  meta.length = now + 100;

  return meta;
}

double input_vorbis::length()
{
  return _length;
}

pcm_info input_vorbis::info()
{
  pcm_info info;
  info.rate = _info->rate;
  info.channels = _info->channels;
  info.endian = this->_endian == 0 ? PCM_LE : PCM_BE;
  info.bps = 16;
  return info;
}

pcm_packet::ptr input_vorbis::readsome()
{
  char buffer[1024 * 8];

  int r = ov_read(_oggfile.get(), buffer, sizeof(buffer), this->_endian, 2, 1, &this->_bitstream);

  pcm_packet::ptr pcm;

  if (r > 0) {
    pcm.reset(new pcm_packet(buffer, r));
  }
  else if (r == 0) {
    return pcm;
  }
  else {
    throw input_error("failed to read stream");
  }

  /*pcm->info.channels = _info->channels;
  pcm->info.rate = _info->rate;
  pcm->info.endian = this->_endian == 0 ? PCM_LE : PCM_BE;*/

  return pcm;
}

void initialize_vorbis() {
  /* nothing to do */
}
