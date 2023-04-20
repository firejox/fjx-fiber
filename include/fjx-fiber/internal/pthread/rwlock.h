#ifndef fjx_fiber_internal_rwlock_h
#define fjx_fiber_internal_rwlock_h

#include <pthread.h>

struct fjx_rwlock__ {
    pthread_rwlock_t impl;
};
typedef struct fjx_rwlock__ fjx_rwlock;

void fjx_rwlock_init(fjx_rwlock *);
void fjx_rwlock_lockr(fjx_rwlock *);
void fjx_rwlock_unlockr(fjx_rwlock *);
void fjx_rwlock_lockw(fjx_rwlock *);
void fjx_rwlock_unlockw(fjx_rwlock *);
void fjx_rwlock_destroy(fjx_rwlock *);

#endif
