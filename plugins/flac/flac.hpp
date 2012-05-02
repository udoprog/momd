#ifndef __INPUT_FLAC_HPP__
#define __INPUT_FLAC_HPP__

#include <FLAC++/decoder.h>

#include "input.hpp"

#include <string>
#include <memory>

class pcm_meta;
class pcm_info;
class pcm_packet;

void initialize_flac();

//class OurDecoder: public FLAC::Decoder::File {
//public:
	//OurDecoder(FILE *f_): FLAC::Decoder::File(), f(f_) { }
//protected:
	//FILE *f;

	//virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
	//virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
	//virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);
//};
class input_flac : public input_base, public FLAC::Decoder::File {
public:
  input_flac();
  ~input_flac();

  virtual void open(std::string path);
  virtual void close();
  virtual void seek(double pos);
  virtual pcm_meta tell();
  virtual double length();

  virtual pcm_info info();
  virtual pcm_packet_ptr readsome();

  virtual FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
  virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
  virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);
private:
  char* _buffer;
  int _size;
  FLAC__uint64 _total_samples;
  unsigned _sample_rate;
  unsigned _channels;
  unsigned _bps;
};

#endif /* __INPUT_FLAC_HPP__ */
