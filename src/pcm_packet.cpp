#include "pcm_packet.hpp"

#include <cstring>

pcm_packet::pcm_packet(pcm_format format, const char* data, int size)
{
  std::string string_data(data, size);
  this->_format = format;
  this->_data = string_data;
}

pcm_packet::pcm_packet()
{
}

pcm_format pcm_packet::format()
{
    return _format;
}

const char* pcm_packet::data()
{
    return _data.c_str();
}

size_t pcm_packet::size()
{
    return _data.size();
}

pcm_packet::~pcm_packet()
{
}
