#ifndef __INPUT_ERROR__
#define __INPUT_ERROR__

#include <stdexcept>

class input_error : public std::runtime_error
{
public:
  input_error(const char* message);
};

#endif /* __INPUT_ERROR__ */
