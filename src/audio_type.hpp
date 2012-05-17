#ifndef __AUDIO_TYPE__
#define __AUDIO_TYPE__

#include <string>

enum audio_type {
    AUDIO_TYPE_UNKNOWN = 0x0,
    AUDIO_TYPE_MP3 = 0x1,
    AUDIO_TYPE_FLAC = 0x2,
    AUDIO_TYPE_M4A = 0x3,
    AUDIO_TYPE_OGG = 0x4,
    AUDIO_TYPE_MAX = 0x5
};

audio_type detect_audio_type(std::string path);
const char* audio_type_name(audio_type type);

#endif /*__AUDIO_TYPE__*/
