#ifndef fjx_fiber_internal_thread_mutex_h
#define fjx_fiber_internal_thread_mutex_h

#include <pthread.h>
#include "../visibility.h"

struct fjx_thread_mutex__;
typedef struct fjx_thread_mutex__ fjx_thread_mutex;

struct fjx_thread_mutex__ {
    pthread_mutex_t impl;
};

DLL_LOCAL void fjx_thread_mutex_init(fjx_thread_mutex *);
DLL_LOCAL void fjx_thread_mutex_lock(fjx_thread_mutex *);
DLL_LOCAL void fjx_thread_mutex_unlock(fjx_thread_mutex *);
DLL_LOCAL void fjx_thread_mutex_destroy(fjx_thread_mutex *);

#endif
