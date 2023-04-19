#ifndef fjx_fiber_internal_spinlock_h
#define fjx_fiber_internal_spinlock_h

#include <pthread.h>

struct __fjx_spinlock {
    pthread_spinlock_t impl;
};
typedef struct __fjx_spinlock fjx_spinlock;

void fjx_spinlock_init(fjx_spinlock *);
void fjx_spinlock_lock(fjx_spinlock *);
void fjx_spinlock_unlock(fjx_spinlock *);
void fjx_spinlock_destroy(fjx_spinlock *);

#endif
