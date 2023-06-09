#ifndef fjx_fiber_internal_thread_semaphore_impl_h
#define fjx_fiber_internal_thread_semaphore_impl_h

#include "../visibility.h"
#include <threads.h>

struct fjx_thread_semaphore__ {
    mtx_t   m;
    cnd_t   c;
    unsigned count;
};
typedef struct fjx_thread_semaphore__ fjx_thread_semaphore;

DLL_LOCAL void fjx_thread_semaphore_init(fjx_thread_semaphore *, unsigned);
DLL_LOCAL void fjx_thread_semaphore_wait(fjx_thread_semaphore *);
DLL_LOCAL void fjx_thread_semaphore_signal(fjx_thread_semaphore *);
DLL_LOCAL void fjx_thread_semaphore_destroy(fjx_thread_semaphore *);

#endif
