#ifndef fjx_fiber_internal_thread_cond_h
#define fjx_fiber_internal_thread_cond_h

#include <threads.h>
#include "../visibility.h"
#include "./mutex.h"

struct fjx_thread_cond__ {
    cnd_t impl;
};

typedef struct fjx_thread_cond__ fjx_thread_cond;

DLL_LOCAL void fjx_thread_cond_init(fjx_thread_cond *);
DLL_LOCAL void fjx_thread_cond_wait(fjx_thread_cond *, fjx_thread_mutex *);
DLL_LOCAL void fjx_thread_cond_timedwait(fjx_thread_cond *, fjx_thread_mutex *, struct timespec *);
DLL_LOCAL void fjx_thread_cond_siganl(fjx_thread_cond *);
DLL_LOCAL void fjx_thread_cond_broadcast(fjx_thread_cond *);
DLL_LOCAL void fjx_thread_cond_destroy(fjx_thread_cond *);

#endif
