#ifndef __PCM_PACKET__
#define __PCM_PACKET__

#include "pcm_format.hpp"

#include <memory>

/**
 * pcm_packets are expected to be 16 BIT per sample.
 */
class pcm_packet
{
private:
  pcm_format _format;
  std::string _data;
public:
  typedef std::shared_ptr<pcm_packet> ptr;

  pcm_packet(pcm_format format, const char* data, int size);
  pcm_packet();
  ~pcm_packet();

  pcm_format format();
  const char* data();
  size_t size();

  MSGPACK_DEFINE(_format, _data)
};

#endif /* __PCM_PACKET__ */
