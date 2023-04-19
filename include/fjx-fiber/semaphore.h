#ifndef fjx_fiber_semaphore_h
#define fjx_fiber_semaphore_h

#include "./utils.h"

struct __fjx_fiber_semaphore;
typedef struct __fjx_fiber_semaphore fjx_fiber_semaphore;

fjx_fiber_semaphore *fiber_semaphore_create(unsigned int);
void fiber_semaphore_signal(fjx_fiber_scheduler*, fjx_fiber_semaphore*);
void fiber_semaphore_wait(fjx_fiber_scheduler*, fjx_fiber_semaphore*);
void fiber_semaphore_destroy(fjx_fiber_scheduler*, fjx_fiber_semaphore*);

#endif
