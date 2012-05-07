#include "main_service.hpp"

#include "input.hpp"
#include "output.hpp"

#include "output_service.hpp"
#include "decoder_service.hpp"
#include "logger_service.hpp"
#include "medialib_service.hpp"
#include "plugin.hpp"
#include "output_base.hpp"

#include "dlopen.hpp"

#include <libconfig.h++>
#include <zmq.hpp>

#include <thread>
#include <stdexcept>

#include <iostream>

void output_event_loop(zmq::context_t* context, libconfig::Config* config)
{
    output_base_ptr output;

    if (config->exists("output")) {
        libconfig::Setting& output_config = config->lookup("output");

        std::string type;

        if (output_config.lookupValue("type", type)) {
            output = open_output(type);
            output->setup(output_config);
            output->open();
        }
    }

    output_service service(context, output);
    service.run();
}

void decoder_event_loop(zmq::context_t* context, libconfig::Config* config)
{
    decoder_service service(context);
    service.run();
}

void medialib_event_loop(zmq::context_t* context, libconfig::Config* config)
{
    medialib_service service(context);
    service.add_song("test.mp3");
    service.run();
}

void logger_event_loop(logger_service* service)
{
    service->run();
}

void read_config(libconfig::Config& config, char* path)
{
    FILE* conf = fopen(path, "r");

    if (conf == NULL) {
        throw std::runtime_error("could not open configuration");
    }

    config.read(conf);

    fclose(conf);
}

void main_event_loop(libconfig::Config& config, zmq::context_t& context)
{
    logger_service logger(&context);
    std::thread logger_thread(logger_event_loop, &logger);

    main_service main(context);

    std::thread decoder_thread(decoder_event_loop, &context, &config);
    std::thread medialib_thread(medialib_event_loop, &context, &config);
    std::thread output_thread(output_event_loop, &context, &config);

    main.run();

    /* join all threads */
    std::cerr << "MAIN: Joining Threads" << std::endl;
    decoder_thread.join();
    medialib_thread.join();
    output_thread.join();

    logger.stop();
}

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

void traceback_handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 2);
    exit(1);
}

void kill_handler(int sig) {
    std::cout << "KILLED" << std::endl;
    exit(1);
}

int main(int argc, char* argv[])
{
    signal(SIGSEGV, traceback_handler);
    signal(SIGABRT, traceback_handler);
    signal(SIGINT, kill_handler);

    const char* program_name = argv[0];

    if (argc < 2) {
        std::cout << "Usage: " << program_name << " <conf>" << std::endl;
        return 1;
    }

    libconfig::Config config;
    read_config(config, argv[1]);

    zmq::context_t context(1);

    /* global initialization of plugins */
    initialize_plugins(config);

    /* global initialization of input/output */
    initialize_input();
    initialize_output();

    main_event_loop(config, context);
    return 0;
}
