#include "flac.hpp"

#include "plugin.hpp"
#include "input_error.hpp"
#include "pcm_meta.hpp"
#include "pcm_packet.hpp"
#include "pcm_position.hpp"

input_base* flac__new();
void flac__initialize();
bool flac__check(audio_type type);

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

bool flac__check(audio_type type)
{
    switch (type) {
        case AUDIO_TYPE_FLAC:
            return true;
        default:
            return false;
    }
}

input_flac::input_flac()
    : buffer(NULL)
{
}

input_flac::~input_flac()
{
    if (buffer) {
        delete [] buffer;
        buffer = NULL;
    }
}

void input_flac::open(std::string path, pcm_format format)
{
  FLAC__StreamDecoderInitStatus status = init(path);

  if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    throw new input_error("failed to initialize decoder");
  }

  process_until_end_of_metadata();

  current_format = format;
}

void input_flac::close()
{
}

void input_flac::seek(double pos)
{
  seek_absolute((pos * current_format.rate));
}

pcm_position input_flac::tell()
{
  pcm_position position;
  position.current = 0;
  position.length = 100;
  return position;
}

double input_flac::length()
{
  return total_samples / current_format.rate;
}

pcm_packet::ptr input_flac::readsome()
{
  if (!buffer) {
    throw input_error("buffer has not been initialized (no metadata section?)");
  }

  pcm_packet::ptr pcm;

  size_t size = 0;
  
  while (size == 0) {
    if (get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
      return pcm;
    }

    if (get_state() != FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {
      throw new input_error(get_state().as_cstring());
    }

    if (!process_single()) {
      throw new input_error("failed to process single");
    }
  }

  pcm.reset(new pcm_packet(current_format, buffer, size));
  /*pcm->info.channels = 2;
  pcm->info.rate = 44100;
  pcm->info.endian = PCM_LE;*/
  return pcm;
}

void initialize_flac() {
}

FLAC__StreamDecoderWriteStatus input_flac::write_callback(const ::FLAC__Frame *frame, const FLAC__int32* const input_buffer[])
{
  unsigned int pos = 0;

  for(unsigned int i = 0; i < frame->header.blocksize; i++) {
    for (unsigned int j = 0; j < current_format.channels; j++) {
      FLAC__int32 channel = input_buffer[j][i];
      for (unsigned short bps = 0; bps < current_format.bps; bps += 8) {
        buffer[pos++] = ((char*)&channel)[bps/8];
      }
    }
  }

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void input_flac::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
      total_samples = metadata->data.stream_info.total_samples;
      current_format.rate = metadata->data.stream_info.sample_rate;
      current_format.channels = metadata->data.stream_info.channels;
      current_format.bps = metadata->data.stream_info.bits_per_sample;

      unsigned max = metadata->data.stream_info.max_blocksize;

      if (buffer) {
          delete [] buffer;
          buffer = NULL;
      }

      buffer = new char[max * current_format.channels * current_format.bps / 8];
  }
}

void input_flac::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
}
