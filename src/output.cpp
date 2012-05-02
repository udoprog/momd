#include "output.hpp"

#include "output/pulse.hpp"

output_error::output_error(const char* message)
  : message(message), message_extra(NULL)
{
}

output_error::output_error(const char* message, const char* message_extra)
  : message(message), message_extra(message_extra)
{
}

output_error::~output_error() throw()
{
}

const char* output_error::what() const throw()
{
  return this->message;
}

const char* output_error::extra() const
{
  return this->message_extra;
}

output_base_ptr open_output(std::string name)
{
  output_base_ptr base;

  if (std::string("pulse").compare(name) == 0) {
    base.reset(new output_pulse());
  }
  else {
    throw output_error("unsupported output type");
  }

  return base;
}

void output_base::set_name(std::string name)
{
  _name = name;
}

std::string output_base::name()
{
  return _name;
}

void initialize_output()
{
}
