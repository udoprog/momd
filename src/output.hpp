#ifndef __OUTPUT_HPP__
#define __OUTPUT_HPP__

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <libconfig.h++>

struct pcm_packet;
struct pcm_info;

class output_error : public std::exception
{
public:
  output_error(const char* message);
  output_error(const char* message, const char* extra);
  virtual ~output_error() throw();
  virtual const char* what() const throw();
  const char* extra() const;
private:
  const char* message;
  const char* message_extra;
};

class output_base {
public:
  typedef boost::shared_ptr<pcm_packet> pcm_packet_ptr;

  virtual void setup(libconfig::Setting&) = 0;
  virtual void open() = 0;
  virtual bool is_open() = 0;
  virtual void close() = 0;
  virtual void write(pcm_packet_ptr) = 0;
  virtual pcm_info info() = 0;

  void set_name(std::string);
  std::string name();
private:
  std::string _name;
};

typedef boost::shared_ptr<output_base> output_base_ptr;

output_base_ptr open_output(std::string name);

void initialize_output();

#endif /* __OUTPUT_HPP__ */
