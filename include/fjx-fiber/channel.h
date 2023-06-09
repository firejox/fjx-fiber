#ifndef fjx_fiber_channel_h
#define fjx_fiber_channel_h

#include <stddef.h>

struct fjx_fiber_channel__;
typedef struct fjx_fiber_channel__ fjx_fiber_channel;

enum {
    CHANNEL_SUCCESS = 0,
    CHANNEL_CLOSED = 1,
    CHANNEL_DESTROY = 2
};

fjx_fiber_channel *fiber_channel_create(size_t);
int fiber_channel_send(fjx_fiber_channel*, void*);
int fiber_channel_receive(fjx_fiber_channel*, void*);
void fiber_channel_close(fjx_fiber_channel*);
void fiber_channel_destroy(fjx_fiber_channel*);

#endif
