#ifndef fjx_fiber_internal_qspinlock_h
#define fjx_fiber_internal_qspinlock_h

#include "./mcs-lock.h"

typedef fjx_mcslock             fjx_qspinlock;
typedef fjx_mcslock_ptr         fjx_qspinlock_ptr;
typedef fjx_mcslock_atomic_ptr  fjx_qspinlock_atomic_ptr;

#define FJX_QSPINLOCK_INIT ((fjx_qspinlock){})

static inline void fjx_qspinlock_init(fjx_qspinlock *lock) {
    *lock = FJX_QSPINLOCK_INIT;
}

void fjx_qspinlock_lock(fjx_qspinlock *);

void fjx_qspinlock_unlock(fjx_qspinlock *);

#define fjx_qspinlock_destroy(lck) ((void)(0))

#endif
