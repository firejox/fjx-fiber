#ifndef fjx_fiber_mutex_h
#define fjx_fiber_mutex_h

#include "./utils.h"

struct __fjx_fiber_mutex;
typedef struct __fjx_fiber_mutex fjx_fiber_mutex;

fjx_fiber_mutex *fiber_mutex_create(void);
void fiber_mutex_lock(fjx_fiber_scheduler*, fjx_fiber_mutex*);
void fiber_mutex_unlock(fjx_fiber_scheduler*, fjx_fiber_mutex*);
void fiber_mutex_destroy(fjx_fiber_scheduler*, fjx_fiber_mutex*);

#endif
