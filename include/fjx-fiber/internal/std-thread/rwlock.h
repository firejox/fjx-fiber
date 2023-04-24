#ifndef fjx_fiber_internal_rwlock_h
#define fjx_fiber_internal_rwlock_h

#include <threads.h>
#include "../visibility.h"

struct fjx_rwlock__ {
    mtx_t   m;
    cnd_t   cond;
    int     w_waited;
    int     r_reading;
    int     on_write;
};
typedef struct fjx_rwlock__ fjx_rwlock;

DLL_LOCAL void fjx_rwlock_init(fjx_rwlock *);
DLL_LOCAL void fjx_rwlock_lockr(fjx_rwlock *);
DLL_LOCAL void fjx_rwlock_unlockr(fjx_rwlock *);
DLL_LOCAL void fjx_rwlock_lockw(fjx_rwlock *);
DLL_LOCAL void fjx_rwlock_unlockw(fjx_rwlock *);
DLL_LOCAL void fjx_rwlock_destroy(fjx_rwlock *);

#endif
