#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#include <memory>

class pcm_meta;
class pcm_packet;
class pcm_info;
class input_base;

typedef void(*initializer_func)();
extern const initializer_func init[];

void initialize_input();

typedef std::shared_ptr<input_base> input_base_ptr;

input_base_ptr open_path(std::string path);

#endif /* __INPUT_HPP__ */
