#ifndef fjx_fiber_internal_rwlock_h
#define fjx_fiber_internal_rwlock_h

#include <threads.h>

struct __fjx_rwlock {
    mtx_t   m;
    cnd_t   cond;
    int     w_waited;
    int     r_reading;
    int     on_write;
};
typedef struct __fjx_rwlock fjx_rwlock;

void fjx_rwlock_init(fjx_rwlock *);
void fjx_rwlock_lockr(fjx_rwlock *);
void fjx_rwlock_unlockr(fjx_rwlock *);
void fjx_rwlock_lockw(fjx_rwlock *);
void fjx_rwlock_unlockw(fjx_rwlock *);
void fjx_rwlock_destroy(fjx_rwlock *);

#endif
