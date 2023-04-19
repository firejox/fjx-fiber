#ifndef fjx_fiber_fiber_h
#define fjx_fiber_fiber_h

#include "./utils.h"

void fiber_spawn(fjx_fiber_scheduler *, void (*)(void *), void *);
void fiber_yield(fjx_fiber_scheduler *);

#endif

