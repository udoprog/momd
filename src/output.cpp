#include "output.hpp"
#include "output_error.hpp"
#include "output_base.hpp"

#include "plugin.hpp"

#include <iostream>

output_base_ptr open_output(std::string name)
{
    output_base_ptr output;

    std::vector<plugin_spec*>::iterator iter = output_plugins.begin();
   
    while (iter != output_plugins.end()) {
        plugin_spec* spec = *(iter++);

        if (name.compare(spec->name) != 0) {
            continue;
        }

        output.reset(spec->data.output.output_new());
    }

    if (!output) {
        throw output_error("unsupported output type");
    }

    return output;
}

void initialize_output()
{
    std::vector<plugin_spec*>::iterator iter = output_plugins.begin();

    while (iter != output_plugins.end()) {
        plugin_spec* spec = *(iter++);
        spec->data.output.output_initialize();
    }
}
