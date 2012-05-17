#ifndef __PCM_FORMAT__
#define __PCM_FORMAT__

#include <msgpack.hpp>
#include <memory>

/**
 * All pcm formats, native endian type.
 */
enum pcm_format_encoding
{
    PCM_SIGNED_16 = 0,
    PCM_SIGNED_32 = 1,
    PCM_FLOAT_32 = 2
};

class pcm_format
{
public:
  typedef std::shared_ptr<pcm_format> ptr;

  /* frequency rate */
  long int rate;

  /* number of channels */
  unsigned short channels;

  /* encoding */
  int encoding;

  /* bits per segment */
  unsigned short bps;

  /* compare to pcm_format functions */
  bool operator!=(const pcm_format& other) const;
  bool operator==(const pcm_format& other) const;

  MSGPACK_DEFINE(rate, channels, encoding, bps)
};

#endif /* __PCM_FORMAT__ */
