#ifndef fjx_fiber_internal_mcsh_lock_h
#define fjx_fiber_internal_mcsh_lock_h

#include <stdatomic.h>

struct fjx_mcshlock__ ;
typedef struct fjx_mcshlock__ fjx_mcshlock;

struct fjx_mcshlock__ {
    _Atomic(void *) tail;
    void *msg;
    int holding;
};

static inline void fjx_mcshlock_init(fjx_mcshlock *lock) {
    atomic_init(&lock->tail, NULL);
    lock->msg = NULL;
    lock->holding = 0;
}

void fjx_mcshlock_lock(fjx_mcshlock *);
void fjx_mcshlock_unlock(fjx_mcshlock *);
#define fjx_mcshlock_destroy(lck) ((void)0)

#endif
