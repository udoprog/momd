#include "vorbis.hpp"

#include "plugin.hpp"
#include "input_error.hpp"
#include "pcm_meta.hpp"
#include "pcm_packet.hpp"
#include "pcm_format.hpp"
#include "pcm_position.hpp"

#include <vorbis/vorbisfile.h>

input_base* vorbis__new();
void vorbis__initialize();
bool vorbis__check(audio_type type);

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

bool vorbis__check(audio_type type) {
    switch (type) {
    case AUDIO_TYPE_OGG:
        return true;
    default:
        return false;
    }
}

void to_vorbisfile_encoding(int encoding, int& bigendianp, int& word, int& sgned)
{
    switch (encoding) {
    case PCM_SIGNED_16:
        bigendianp = 1;
        word = 2;
        sgned = 1;
        break;
    case PCM_SIGNED_32:
        throw input_error("vorbisfile: does not support PCM_32_SIGNED");
    case PCM_FLOAT_32:
        throw input_error("vorbisfile: does not support PCM_FLOAT_32");
    default:
        throw input_error("vorbisfile: unsupported encoding");
    }
}

input_vorbis::input_vorbis()
  : vf_bitstream(0), vf_length(0.0)
{
    vf_oggfile.reset(new OggVorbis_File);
}

input_vorbis::~input_vorbis()
{
}

void input_vorbis::open(std::string path, pcm_format format)
{
    FILE* fd = fopen(path.c_str(), "rb");

    if (fd == NULL) {
      throw input_error("vorbisfile: failed to open file");
    }

    to_vorbisfile_encoding(format.encoding, bigendianp, word, sgned);

    ov_open(fd, vf_oggfile.get(), NULL, 0);

    vorbis_info* info = ov_info(vf_oggfile.get(), -1);

    if (info == NULL) {
        throw input_error("vorbisfile: failed to get vorbis file info");
    }

    vf_info.reset(info);

    this->vf_length = ov_time_total(vf_oggfile.get(), -1);

    current_format = format;
}

void input_vorbis::close()
{
    ov_clear(vf_oggfile.get());
}

void input_vorbis::seek(double pos)
{
  ov_time_seek(vf_oggfile.get(), pos);
}

pcm_position input_vorbis::tell()
{
  double now = ov_time_tell(vf_oggfile.get());

  pcm_position position;
  position.current = now;
  position.length = vf_length;
  return position;
}

double input_vorbis::length()
{
  return vf_length;
}

pcm_packet::ptr input_vorbis::readsome()
{
  char buffer[1024 * 8];

  int r = ov_read(vf_oggfile.get(),
          buffer, sizeof(buffer),
          bigendianp, word, sgned,
          &this->vf_bitstream);

  pcm_packet::ptr pcm;

  if (r > 0) {
    pcm.reset(new pcm_packet(current_format, buffer, r));
  }
  else if (r == 0) {
    return pcm;
  }
  else {
    throw input_error("failed to read stream");
  }

  /*pcm->info.channels = info->channels;
  pcm->info.rate = info->rate;
  pcm->info.endian = this->_endian == 0 ? PCM_LE : PCM_BE;*/

  return pcm;
}

void initialize_vorbis() {
  /* nothing to do */
}
