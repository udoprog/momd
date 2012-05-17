#ifndef __INPUT_FLAC_HPP__
#define __INPUT_FLAC_HPP__

#include "input_base.hpp"

#include <FLAC++/decoder.h>
#include <string>
#include <memory>

class pcm_meta;
class pcm_format;
class pcm_packet;
class pcm_position;

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

  virtual void open(std::string path, pcm_format);
  virtual void close();
  virtual void seek(double pos);
  virtual pcm_position tell();
  virtual double length();

  virtual std::shared_ptr<pcm_packet> readsome();

  virtual FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *, const FLAC__int32 * const[]);
  virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
  virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);
private:
  char* buffer;
  FLAC__uint64 total_samples;
  pcm_format current_format;
};

#endif /* __INPUT_FLAC_HPP__ */
