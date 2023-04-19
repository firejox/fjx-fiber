#ifndef fjx_fiber_scheduler_h
#define fjx_fiber_scheduler_h

#include "./utils.h"

fjx_fiber_scheduler *fiber_scheduler_create(int);

void fiber_scheduler_destroy(fjx_fiber_scheduler *);

#endif
