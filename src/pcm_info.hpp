#ifndef __PCM_INFO__
#define __PCM_INFO__

#include <memory>

#define PCM_LE 0
#define PCM_BE 1

class pcm_info
{
public:
  typedef std::shared_ptr<pcm_info> ptr;

  /* frequency rate */
  long int rate;

  /* number of channels */
  int channels;

  /* endianness, PCM_LE or PCM_BE */
  int endian;

  /* bits per segment */
  int bps;

  /* compare to pcm_info functions */
  bool operator!=(const pcm_info& other) const;
  bool operator==(const pcm_info& other) const;
};

#endif /* __PCM_INFO__ */
