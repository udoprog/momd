#ifndef __IO_HPP__
#define __IO_HPP__

#include <boost/shared_ptr.hpp>

#include <cstring>

#define PCM_LE 0
#define PCM_BE 1

class pcm_info {
public:
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

struct pcm_meta {
  double current;
  double length;
};

/**
 * pcm_packets are expected to be 16 BIT per sample.
 */
class pcm_packet {
public:
  pcm_packet(const char* data, int size);
  ~pcm_packet();

  const char* data();
  int size();
private:
  const char* _data;
  int _size;
};

typedef boost::shared_ptr<pcm_packet> pcm_packet_ptr;

#endif /* __IO_HPP__ */
