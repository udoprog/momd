#ifndef __OUTPUT_ERROR__
#define __OUTPUT_ERROR__

#include <stdexcept>

class output_error : public std::runtime_error
{
public:
  output_error(const char* message);
};

#endif /* __OUTPUT_ERROR__ */
