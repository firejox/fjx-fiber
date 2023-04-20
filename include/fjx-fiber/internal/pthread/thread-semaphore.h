#ifndef fjx_fiber_internal_thread_semaphore_impl_h
#define fjx_fiber_internal_thread_semaphore_impl_h

#include "../visibility.h"

#include <semaphore.h>

struct fjx_thread_semaphore__ {
    sem_t impl;
};
typedef struct fjx_thread_semaphore__ fjx_thread_semaphore;

DLL_LOCAL void fjx_thread_semaphore_init(fjx_thread_semaphore *, unsigned);
DLL_LOCAL void fjx_thread_semaphore_wait(fjx_thread_semaphore *);
DLL_LOCAL void fjx_thread_semaphore_signal(fjx_thread_semaphore *);
DLL_LOCAL void fjx_thread_semaphore_destroy(fjx_thread_semaphore *);

#endif
