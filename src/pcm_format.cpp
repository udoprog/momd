#include "pcm_format.hpp"

bool pcm_format::operator!=(const pcm_format& other) const
{
  return rate != other.rate
      || channels != other.channels
      || encoding != other.encoding
      || bps != other.bps;
}

bool pcm_format::operator==(const pcm_format& other) const
{
  return rate == other.rate
      && channels == other.channels
      && encoding == other.encoding
      && bps == other.bps;
}
