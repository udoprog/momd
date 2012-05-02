#include "input/flac.hpp"
#include "io.hpp"

#include <stdio.h>

#include <boost/bind.hpp>

input_flac::input_flac()
{
}

input_flac::~input_flac()
{
}

void input_flac::open(fs::path path)
{
  FLAC__StreamDecoderInitStatus status = init(path.string());

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

  pcm.reset(new pcm_packet(_buffer.get(), _size));
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

  char *b = _buffer.get();

  for(unsigned int i = 0; i < frame->header.blocksize; i++) {
    for (unsigned int j = 0; j < _channels; j++) {
      FLAC__int32 channel = buffer[j][i];
      for (unsigned int bps = 0; bps < _bps; bps += 8) {
        b[pos++] = ((char*)&channel)[bps/8];
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
    _buffer.reset(new char[max * _channels * _bps / 8]);
  }
}

void input_flac::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
}
