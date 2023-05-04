#ifndef fjx_fiber_internal_mcsh_lock_h
#define fjx_fiber_internal_mcsh_lock_h

struct fjx_mcshlock__ ;
typedef struct fjx_mcshlock__ fjx_mcshlock;

struct fjx_mcshlock__ {
    _Atomic(fjx_mcshlock *) tail;
    fjx_mcshlock *msg;
};

#define FJX_MCSH_LOCK_INIT ((fjx_mcshlock){})

static inline void fjx_mcshlock_init(fjx_mcshlock *lock) {
    *lock = FJX_MCSH_LOCK_INIT;
}

void fjx_mcshlock_lock(fjx_mcshlock *);
void fjx_mcshlock_unlock(fjx_mcshlock *);
#define fjx_mcshlock_destroy(lck) ((void)0)

#endif
