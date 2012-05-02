#ifndef __OUTPUT_EVENT_LOOP__
#define __OUTPUT_EVENT_LOOP__

#include <algorithm>
#include <libconfig.h++>
#include <zmq.hpp>

void output_event_loop(zmq::context_t* context, libconfig::Config* config);

#endif /*  __OUTPUT_EVENT_LOOP__ */
