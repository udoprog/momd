#ifndef __DECODER_EVENT_LOOP__
#define __DECODER_EVENT_LOOP__

#include <algorithm>
#include <zmq.hpp>

void decoder_event_loop(zmq::context_t* context);

#endif /*  __DECODER_EVENT_LOOP__ */
