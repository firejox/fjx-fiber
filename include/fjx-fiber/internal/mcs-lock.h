#ifndef fjx_fiber_internal_mcslock_h
#define fjx_fiber_internal_mcslock_h

#include <stdatomic.h>

struct fjx_mcslock__;
typedef struct fjx_mcslock__ fjx_mcslock;

struct fjx_mcslock__ {
    fjx_mcslock *next;
    int locked;
};

typedef _Atomic(fjx_mcslock *) fjx_mcslock_shared;

void fjx_mcslock_init(fjx_mcslock_shared *);
void fjx_mcslock_lock(fjx_mcslock_shared *, fjx_mcslock *);
void fjx_mcslock_unlock(fjx_mcslock_shared *, fjx_mcslock *);
#define fjx_mcslock_destroy(lock) ((void)(0))

#endif
