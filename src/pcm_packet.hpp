#ifndef __PCM_PACKET__
#define __PCM_PACKET__

#include <memory>

/**
 * pcm_packets are expected to be 16 BIT per sample.
 */
class pcm_packet
{
public:
  typedef std::shared_ptr<pcm_packet> ptr;

  pcm_packet(const char* data, int size);
  ~pcm_packet();

  const char* data();
  int size();
private:
  const char* _data;
  int _size;
};

#endif /* __PCM_PACKET__ */
