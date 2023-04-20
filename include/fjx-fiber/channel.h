#ifndef fjx_fiber_channel_h
#define fjx_fiber_channel_h

#include "./utils.h"

struct __fjx_fiber_channel;
typedef struct __fjx_fiber_channel fjx_fiber_channel;

#define CHANNEL_SUCCESS 0
#define CHANNEL_CLOSED  1
#define CHANNEL_DESTROY 2

fjx_fiber_channel *fiber_channel_create(size_t);
int fiber_channel_send(fjx_fiber_scheduler*, fjx_fiber_channel*, void*);
int fiber_channel_receive(fjx_fiber_scheduler*, fjx_fiber_channel*, void*);
void fiber_channel_close(fjx_fiber_scheduler*, fjx_fiber_channel*);
void fiber_channel_destroy(fjx_fiber_scheduler*, fjx_fiber_channel*);

#endif
