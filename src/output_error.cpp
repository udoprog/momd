#include "output_error.hpp"

output_error::output_error(const char* message)
 : std::runtime_error(message)
{
}
