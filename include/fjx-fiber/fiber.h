#ifndef fjx_fiber_fiber_h
#define fjx_fiber_fiber_h

void fiber_spawn(void (* /* fiber_entry */)(void *), void * /* data */);
void fiber_yield(void);

#endif

