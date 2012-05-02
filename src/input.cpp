#include "input.hpp"
#include "plugin.hpp"

#include <vector>
#include <boost/filesystem.hpp>

input_error::input_error(const char* message)
  : message(message), message_extra(NULL)
{
}

input_error::input_error(const char* message, const char* message_extra)
  : message(message), message_extra(message_extra)
{
}

input_error::~input_error() throw()
{
}

const char* input_error::what() const throw()
{
  return this->message;
}

const char* input_error::extra() const
{
  return this->message_extra;
}

void initialize_input()
{
    std::vector<plugin_spec*>::iterator iter = input_plugins.begin();
   
    while (iter != input_plugins.end()) {
        plugin_spec* spec = *(iter++);
        spec->data.input.input_initialize();
    }
}

input_base_ptr open_path(std::string file_path)
{
    using boost::filesystem::extension;
    using boost::filesystem::path;

    path p = file_path;

    std::vector<std::string> parts;

    std::string ext = extension(p);

    input_base_ptr input;

    std::vector<plugin_spec*>::iterator iter = input_plugins.begin();

    while (iter != input_plugins.end()) {
        plugin_spec* spec = *(iter++);

        if (spec->data.input.input_check(p.string().c_str(), ext.c_str())) {
            input.reset(spec->data.input.input_new());
            break;
        }
    }

    if (!input) {
        throw input_error("unsupported file extension");
    }
   
    input->open(file_path);
    return input;
}
