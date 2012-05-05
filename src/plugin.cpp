#include "plugin.hpp"
#include "dlopen.hpp"

#include <boost/filesystem.hpp>
#include <deque>

std::vector<plugin_spec*> input_plugins;
std::vector<plugin_spec*> output_plugins;

void initialize_single_plugin(const char* plugin_path)
{
    dl_t* dl = dl_open(plugin_path);

    if (!dl) {
        std::cerr << "Could not open plugin: " << plugin_path << std::endl;
        return;
    }

    plugin_spec* spec = (plugin_spec*)dl_sym(dl, PLUGIN_SPEC_NAME);

    if (!spec) {
        dl_close(dl);
        std::cerr << "Not a valid plugin: " << plugin_path << std::endl;
        return;
    }

    std::cout << "plugin(" << spec->name << "): " << plugin_path << std::endl;

    switch (spec->type) {
    case OUTPUT_PLUGIN:
        output_plugins.push_back(spec);
        break;
    case INPUT_PLUGIN:
        input_plugins.push_back(spec);
        break;
    default:
        std::cerr << plugin_path << ": unknown plugin type: "
            << int(spec->type) << std::endl;
        break;
    }
}

void initialize_from_array(libconfig::Setting& plugins)
{
    for (int i = 0; i < plugins.getLength(); i++) {
        const char* plugin_path = plugins[i];
        initialize_single_plugin(plugin_path);
    }
}

void initialize_from_string(libconfig::Setting& plugins)
{
    using boost::filesystem::exists;
    using boost::filesystem::directory_iterator;
    using boost::filesystem::path;
    using boost::filesystem::extension;

    std::string plugins_path = plugins;

    std::deque<path> directories;

    directories.push_back(plugins_path);

    while (!directories.empty()) {
        path directory = directories.front();

        directories.pop_front();

        if (!exists(directory)) {
            return;
        }

        directory_iterator end;
        directory_iterator iter(directory);

        for (;iter != end; ++iter)
        {
            if (is_directory(iter->status())) {
                directories.push_front(iter->path());
                continue;
            }

            path p = iter->path();

            if (extension(p).compare(DL_EXTENSION) == 0) {
                initialize_single_plugin(p.string().c_str());
            }
        }
    }
}

void initialize_plugins(libconfig::Config& config)
{
    libconfig::Setting& root = config.getRoot();

    if (!root.exists("plugins")) {
        return;
    }

    libconfig::Setting& plugins = root["plugins"];

    switch (plugins.getType())
    {
    case libconfig::Setting::TypeArray:
        initialize_from_array(plugins);
        break;
    case libconfig::Setting::TypeString:
        initialize_from_string(plugins);
        break;
    default:
        break;
    }
}
