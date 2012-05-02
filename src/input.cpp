#include "input.hpp"

#include "input/mpg123.hpp"
#include "input/vorbis.hpp"
#include "input/flac.hpp"

const initializer_func init[] = {
  &initialize_mpg123,
  &initialize_vorbis,
  &initialize_flac,
  NULL
};

input_error::input_error(const char* message)
  : message(message), message_extra(NULL)
{
}

input_error::input_error(const char* message, const char* message_extra)
  : message(message), message_extra(message_extra)
{
}

input_error::~input_error() throw()
{
}

const char* input_error::what() const throw()
{
  return this->message;
}

const char* input_error::extra() const
{
  return this->message_extra;
}

void initialize_input()
{
  int i = 0;

  while (true) {
    initializer_func f = init[i++];

    if (f == NULL) {
      break;
    }

    f();
  }
}

input_base_ptr open_path(fs::path path)
{
  input_base_ptr base;

  if (std::string(".ogg").compare(path.extension().string()) == 0) {
    base.reset(new input_vorbis());
  }
  else if (std::string(".flac").compare(path.extension().string()) == 0) {
    base.reset(new input_flac());
  }
  else if (std::string(".mp3").compare(path.extension().string()) == 0) {
    base.reset(new input_mpg123());
  }
  else {
    throw input_error("unsupported file extension");
  }

  base->open(path);
  return base;
}
