#include "audio_type.hpp"

#include <fstream>
#include <string>
#include <cstring>

const char* audio_type_names[] = {
    "UNKNOWN",
    "MP3",
    "FLAC",
    "M4A",
    "OGG"
};

struct id3_header {
    char magic[3];
    char version[2];
    char flags;
    char size[4];
};

size_t id3_size(id3_header* header)
{
    return (
      (header->size[0] & 0x7f) >> 21
    | (header->size[1] & 0x7f) >> 14
    | (header->size[2] & 0x7f) >> 7
    | (header->size[3] & 0x7f));
}

const char* audio_type_name(audio_type type)
{
    return audio_type_names[type % AUDIO_TYPE_MAX];
}

/*
 * do file type detection by reading the first 16 bytes of the file and
 * conforming to standard rules..
 */
audio_type detect_audio_type(std::string path)
{
    std::ifstream input(path.c_str());

    char b[16];
    ::memset(b, 0x0, 16);

    input.read(b, 16);

    if (b[0] == 'I'
     && b[1] == 'D'
     && b[2] == '3'
     && b[3] < 0xff
     && b[4] < 0xff
     && b[5] < 0x80
     && b[6] < 0x80
     && b[7] < 0x80
     && b[8] < 0x80
     )
    {
        return AUDIO_TYPE_MP3;
    }

    if (b[0] == 'O'
     && b[1] == 'g'
     && b[2] == 'g'
     && b[3] == 'S'
     && b[4] == 0x00
     && b[5] < 0x08
     )
    {
        return AUDIO_TYPE_OGG;
    }

    if (b[0] == 'f'
     && b[1] == 'L'
     && b[2] == 'a'
     && b[3] == 'C'
     )
    {
        return AUDIO_TYPE_FLAC;
    }

    if (b[0] == 0x00
     && b[1] == 0x00
     && b[2] == 0x00
     && b[3] == 0x20
     && b[4] == 0x66
     && b[5] == 0x74
     && b[6] == 0x79
     && b[7] == 0x70
     && b[8] == 0x4d
     && b[9] == 0x34
     && b[10] == 0x41
     && b[11] == 0x20
     && b[12] == 0x00
     && b[13] == 0x00
     && b[14] == 0x00
     && b[15] == 0x00
     )
    {
        return AUDIO_TYPE_M4A;
    }

    return AUDIO_TYPE_UNKNOWN;
}
