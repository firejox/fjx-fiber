#ifndef fjx_fiber_internal_spinlock_h
#define fjx_fiber_internal_spinlock_h

#include <stdatomic.h>

struct fjx_spinlock__ {
    atomic_int lock;
};
typedef struct fjx_spinlock__ fjx_spinlock;

void fjx_spinlock_init(fjx_spinlock *);
void fjx_spinlock_lock(fjx_spinlock *);
void fjx_spinlock_unlock(fjx_spinlock *);
#define fjx_spinlock_destroy(lock) ((void)0)

#endif
