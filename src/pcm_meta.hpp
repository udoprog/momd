#ifndef __PCM_META__
#define __PCM_META__

#include <string>

struct pcm_meta {
  std::string title;
  std::string artist;
  std::string album;
  double current;
  double length;
};

#endif /* __PCM_META__ */
