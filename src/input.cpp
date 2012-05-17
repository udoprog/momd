#include "input.hpp"
#include "plugin.hpp"
#include "input_error.hpp"
#include "input_base.hpp"
#include "audio_type.hpp"

#include <vector>
#include <boost/filesystem.hpp>

void initialize_input()
{
    std::vector<plugin_spec*>::iterator iter = input_plugins.begin();
   
    while (iter != input_plugins.end()) {
        plugin_spec* spec = *(iter++);
        spec->data.input.input_initialize();
    }
}

input_base_ptr open_path(std::string file_path, pcm_format format)
{
    using boost::filesystem::extension;
    using boost::filesystem::path;

    path p = file_path;

    std::vector<std::string> parts;

    std::string ext = extension(p);

    input_base_ptr input;

    std::vector<plugin_spec*>::iterator iter = input_plugins.begin();

    audio_type type = detect_audio_type(file_path);

    while (iter != input_plugins.end()) {
        plugin_spec* spec = *(iter++);

        if (spec->data.input.input_check(type)) {
            input.reset(spec->data.input.input_new());
            break;
        }
    }

    if (!input) {
        throw input_error("unsupported file extension");
    }
   
    input->open(file_path, format);
    return input;
}
