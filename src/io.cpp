#include "io.hpp"

bool pcm_info::operator!=(const pcm_info& other) const
{
  return rate != other.rate
      || channels != other.channels
      || endian != other.endian
      || bps != other.bps;
}

bool pcm_info::operator==(const pcm_info& other) const
{
  return rate == other.rate
      && channels == other.channels
      && endian == other.endian
      && bps == other.bps;
}

pcm_packet::pcm_packet(const char* data, int size)
{
  char* d = new char[size];
  std::memcpy(d, data, size);
  this->_data = d;
  this->_size = size;
}

const char* pcm_packet::data()
{
    return _data;
}

int pcm_packet::size()
{
    return _size;
}

pcm_packet::~pcm_packet()
{
  delete[] this->_data;
}
