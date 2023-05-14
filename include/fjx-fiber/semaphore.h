#ifndef fjx_fiber_semaphore_h
#define fjx_fiber_semaphore_h

struct fjx_fiber_semaphore__;
typedef struct fjx_fiber_semaphore__ fjx_fiber_semaphore;

fjx_fiber_semaphore *fiber_semaphore_create(unsigned);
fjx_fiber_semaphore *fiber_semaphore_bound_create(unsigned, unsigned);
void fiber_semaphore_signal(fjx_fiber_semaphore*);
void fiber_semaphore_wait(fjx_fiber_semaphore*);
void fiber_semaphore_destroy(fjx_fiber_semaphore*);

#endif
