#include "main_event_loop.hpp"

#include "input.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

#include <iostream>
#include <thread>

void read_config(libconfig::Config& config, char* path)
{
    FILE* conf = fopen(path, "r");

    if (conf == NULL) {
        throw std::exception();
    }

    config.read(conf);

    fclose(conf);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: mpd++ <conf>" << std::endl;
        return 1;
    }

    libconfig::Config config;
    read_config(config, argv[1]);

    zmq::context_t context(1);

    initialize_input();

    main_event_loop(config, context);

    return 0;
}
