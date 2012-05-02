#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#include <exception>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

struct pcm_meta;
struct pcm_packet;
struct pcm_info;

namespace fs = boost::filesystem;

class input_error : public std::exception
{
public:
  input_error(const char* message);
  input_error(const char* message, const char* extra);
  virtual ~input_error() throw();
  virtual const char* what() const throw();
  const char* extra() const;
private:
  const char* message;
  const char* message_extra;
};


class input_base {
public:
  typedef boost::shared_ptr<pcm_packet> pcm_packet_ptr;

  virtual void open(fs::path path) = 0;
  virtual void close() = 0;
  virtual void seek(double pos) = 0;
  virtual pcm_meta tell() = 0;
  virtual double length() = 0;

  virtual pcm_info info() = 0;

  /**
   * @returns Empty pcm_packet_ptr on end of stream.
   */
  virtual pcm_packet_ptr readsome() = 0;
  virtual ~input_base() {
  }
};

typedef boost::shared_ptr<input_base> input_base_ptr;

typedef void(*initializer_func)();
extern const initializer_func init[];

void initialize_input();

input_base_ptr open_path(fs::path path);

#endif /* __INPUT_HPP__ */
