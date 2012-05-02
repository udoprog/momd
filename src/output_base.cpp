#include "output_base.hpp"

void output_base::set_name(std::string name)
{
  _name = name;
}

std::string output_base::name()
{
  return _name;
}
