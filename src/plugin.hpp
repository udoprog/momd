#ifndef __PLUGIN__
#define __PLUGIN__

#include "audio_type.hpp"

#define INPUT_PLUGIN 'I'
#define OUTPUT_PLUGIN 'O'

class output_base;
class input_base;

namespace libconfig {
    class Config;
}

typedef output_base*(*output_new_t)();
typedef void(*output_initialize_t)();
typedef input_base*(*input_new_t)();
typedef bool(*input_check_t)(audio_type);
typedef void(*input_initialize_t)();

struct input_plugin_spec {
    input_initialize_t input_initialize;
    input_check_t input_check;
    input_new_t input_new;
};

struct output_plugin_spec {
    output_initialize_t output_initialize;
    output_new_t output_new;
};

struct plugin_spec {
    const char type;
    const char* name;

    union data_t {
        output_plugin_spec output;
        input_plugin_spec input;
    } data;

    plugin_spec(const char* name, output_plugin_spec output)
        : type(OUTPUT_PLUGIN), name(name)
    {
        data.output = output;
    }

    plugin_spec(const char* name, input_plugin_spec input)
        : type(INPUT_PLUGIN), name(name)
    {
          data.input = input;
    }
};

#include <vector>

extern std::vector<plugin_spec*> output_plugins;
extern std::vector<plugin_spec*> input_plugins;

#define PLUGIN_SPEC_NAME "momd__plugin_spec"

/* declare plugin */
#define DECLARE_OUTPUT_PLUGIN(NAME, SPEC)\
    extern "C" {\
        plugin_spec momd__plugin_spec(NAME, SPEC);\
    }

#define DECLARE_INPUT_PLUGIN(NAME, SPEC)\
    extern "C" {\
        plugin_spec momd__plugin_spec(NAME, SPEC);\
    }

void initialize_plugins(libconfig::Config& config);

#endif /* __PLUGIN__ */
