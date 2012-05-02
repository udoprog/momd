#ifndef __OUTPUT_HPP__
#define __OUTPUT_HPP__

#include <string>
#include <memory>

class output_base;

typedef std::shared_ptr<output_base> output_base_ptr;

output_base_ptr open_output(std::string name);

void initialize_output();

#endif /* __OUTPUT_HPP__ */
