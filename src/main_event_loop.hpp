#ifndef __MAIN_EVENT_LOOP__
#define __MAIN_EVENT_LOOP__

#include <algorithm>
#include <libconfig.h++>
#include <zmq.hpp>

void main_event_loop(libconfig::Config& config, zmq::context_t& context);

#endif /*  __MAIN_EVENT_LOOP__ */
