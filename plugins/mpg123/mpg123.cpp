#include "mpg123.hpp"
#include "io.hpp"
#include "input.hpp"

#include "plugin.hpp"

input_base* mpg123__new();
void mpg123__initialize();
bool mpg123__check(const char* path, const char* ext);

input_plugin_spec mpg123_spec = {
    &mpg123__initialize,
    &mpg123__check,
    &mpg123__new
};

DECLARE_INPUT_PLUGIN("mpg123", mpg123_spec)

input_base* mpg123__new() {
    return new input_mpg123();
}

void mpg123__initialize() {
  int err = mpg123_init();

  if (err != MPG123_OK) {
    throw input_error("failed to initialize mpg123", mpg123_plain_strerror(err));
  }
}

std::string mp3_ext(".mp3");
std::string mp4_ext(".mp4");

bool mpg123__check(const char* path, const char* ext) {
    std::string string_path(path);
    std::string string_ext(ext);

    if (mp3_ext.compare(string_ext) == 0) {
        return true;
    }

    if (mp4_ext.compare(string_ext) == 0) {
        return true;
    }

    return false;
}

input_mpg123::input_mpg123()
  : handle(NULL), _channels(0), _rate(0), _encoding(0),
    _length(0.0)
{
}

input_mpg123::~input_mpg123()
{
  if (handle != NULL) {
    mpg123_delete(handle);
  }
}

void input_mpg123::open(std::string path)
{
  int err;
  int encoding;
  handle = mpg123_new(NULL, &err);

  if (handle == NULL) {
    throw input_error("failed to initialize handle", mpg123_plain_strerror(err));
  }

  if ((err = mpg123_open(handle, path.c_str())) != MPG123_OK) {
    throw input_error("failed to open file", mpg123_plain_strerror(err));
  }

  if ((err = mpg123_getformat(handle, &_rate, &_channels, &encoding)) != MPG123_OK)
  {
    throw input_error("failed to get format", mpg123_plain_strerror(err));
  }

  /* force 16 bit format */
  encoding = MPG123_ENC_SIGNED_16;

  mpg123_format_none(handle);
  mpg123_format(handle, _rate, _channels, encoding);

  double seconds_left;

  if ((err = mpg123_position(handle, 0, 0, NULL, NULL, NULL, &seconds_left)) != MPG123_OK) {
    throw input_error("failed to tell position", mpg123_plain_strerror(err));
  }

  this->_encoding = encoding;
  this->_length = seconds_left;
}

void input_mpg123::close()
{
}

void input_mpg123::seek(double pos)
{
  int err;
  off_t off = mpg123_timeframe(handle, pos);

  if (off < 0) {
    throw input_error("failed to lookup timeframe", mpg123_plain_strerror(off));
  }

  if ((err = mpg123_seek(handle, off, SEEK_SET)) != MPG123_OK) {
    throw input_error("failed to seek position", mpg123_plain_strerror(err));
  }
}

pcm_meta input_mpg123::tell()
{
  int err;
  double seconds;

  if ((err = mpg123_position(handle, 0, 0, NULL, NULL, &seconds, NULL)) != MPG123_OK) {
    throw input_error("failed to tell position", mpg123_plain_strerror(err));
  }

  pcm_meta meta;
  meta.current = seconds;
  meta.length = seconds + 100;

  return meta;
}

double input_mpg123::length()
{
  return _length;
}

pcm_info input_mpg123::info()
{
  pcm_info info;
  info.rate = _rate;
  info.channels = _channels;
  info.endian = PCM_LE;
  info.bps = 16;
  return info;
}

pcm_packet_ptr input_mpg123::readsome()
{
  int err;
  unsigned char buffer[1024 * 8];
  size_t done;

  pcm_packet_ptr pcm;

  if ((err = mpg123_read(handle, buffer, sizeof(buffer), &done)) != MPG123_OK) {
    return pcm;
  }

  pcm.reset(new pcm_packet(reinterpret_cast<char*>(buffer), done));
  /*pcm->info.channels = _channels;
  pcm->info.rate = _rate;
  pcm->info.endian = PCM_LE;*/

  return pcm;
}
