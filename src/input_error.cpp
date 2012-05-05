#include "input_error.hpp"

input_error::input_error(const char* message)
  : std::runtime_error(message)
{
}
