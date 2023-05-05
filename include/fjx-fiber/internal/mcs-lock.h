#ifndef fjx_fiber_internal_mcslock_h
#define fjx_fiber_internal_mcslock_h

struct fjx_mcslock__;
typedef struct fjx_mcslock__ fjx_mcslock;
typedef fjx_mcslock *fjx_mcslock_ptr;
typedef _Atomic(fjx_mcslock_ptr) fjx_mcslock_atomic_ptr;

struct fjx_mcslock__ {
    fjx_mcslock_atomic_ptr next;
    _Atomic(int) locked;
};

#define FJX_MCS_LOCK_INIT ((fjx_mcslock){})

static inline void fjx_mcslock_init(fjx_mcslock *lock) {
    *lock = FJX_MCS_LOCK_INIT;
}

void fjx_mcslock_lock(fjx_mcslock_atomic_ptr *, fjx_mcslock *);
void fjx_mcslock_unlock(fjx_mcslock_atomic_ptr *, fjx_mcslock *);
#define fjx_mcslock_destroy(lock) ((void)(0))

#endif
