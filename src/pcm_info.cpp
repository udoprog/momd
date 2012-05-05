#include "pcm_info.hpp"

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
