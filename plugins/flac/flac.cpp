#include "flac.hpp"
#include "io.hpp"
#include "plugin.hpp"

#include <stdio.h>

input_base* flac__new();
void flac__initialize();
bool flac__check(const char* path, const char* ext);

input_plugin_spec flac_spec = {
    &flac__initialize,
    &flac__check,
    &flac__new
};

DECLARE_INPUT_PLUGIN("flac", flac_spec)

input_base* flac__new() {
    return new input_flac();
}

void flac__initialize() {
}

std::string flac_ext(".flac");

bool flac__check(const char* path, const char* ext) {
    std::string string_path(path);
    std::string string_ext(ext);

    if (flac_ext.compare(string_ext) == 0) {
        return true;
    }

    return false;
}

input_flac::input_flac()
    : _buffer(NULL)
{
}

input_flac::~input_flac()
{
    if (_buffer) {
        delete [] _buffer;
        _buffer = NULL;
    }
}

void input_flac::open(std::string path)
{
  FLAC__StreamDecoderInitStatus status = init(path);

  if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    throw new input_error("failed to initialize decoder");
  }

  process_until_end_of_metadata();
}

void input_flac::close()
{
}

void input_flac::seek(double pos)
{
  seek_absolute((pos * _sample_rate));
}

pcm_meta input_flac::tell()
{
  pcm_meta meta;
  meta.current = 0;
  meta.length = 100;
  return meta;
}

double input_flac::length()
{
  return _total_samples / _sample_rate;
}

pcm_info input_flac::info()
{
  pcm_info info;
  info.rate = _sample_rate;
  info.channels = _channels;
  info.endian = PCM_LE;
  info.bps = 16;
  return info;
}

pcm_packet_ptr input_flac::readsome()
{
  if (!_buffer) {
    throw input_error("buffer has not been initialized (no metadata section?)");
  }

  pcm_packet_ptr pcm;

  _size = 0;
  
  while (_size == 0) {
    if (get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
      return pcm;
    }

    if (get_state() != FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {
      throw new input_error("decoder not syncing", get_state().as_cstring());
    }

    if (!process_single()) {
      throw new input_error("failed to process single");
    }
  }

  pcm.reset(new pcm_packet(_buffer, _size));
  /*pcm->info.channels = 2;
  pcm->info.rate = 44100;
  pcm->info.endian = PCM_LE;*/
  return pcm;
}

void initialize_flac() {
}

FLAC__StreamDecoderWriteStatus input_flac::write_callback(const ::FLAC__Frame *frame, const FLAC__int32* const buffer[])
{
  unsigned int pos = 0;

  for(unsigned int i = 0; i < frame->header.blocksize; i++) {
    for (unsigned int j = 0; j < _channels; j++) {
      FLAC__int32 channel = buffer[j][i];
      for (unsigned int bps = 0; bps < _bps; bps += 8) {
        _buffer[pos++] = ((char*)&channel)[bps/8];
      }
    }
  }

  _size = pos;
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void input_flac::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    _total_samples = metadata->data.stream_info.total_samples;
    _sample_rate = metadata->data.stream_info.sample_rate;
    _channels = metadata->data.stream_info.channels;
    _bps = metadata->data.stream_info.bits_per_sample;
    unsigned max = metadata->data.stream_info.max_blocksize;

    if (_buffer) {
        delete [] _buffer;
        _buffer = NULL;
    }

    _buffer = new char[max * _channels * _bps / 8];
  }
}

void input_flac::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
}
