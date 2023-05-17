#ifndef fjx_fiber_internal_thread_impl_h
#define fjx_fiber_internal_thread_impl_h

#include "../visibility.h"

typedef void (*thread_entry)(void *);

DLL_LOCAL void fjx_thread_spawn(thread_entry, void *);
DLL_LOCAL void fjx_thread_yield(void);

#endif
