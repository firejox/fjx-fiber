#include <stdatomic.h>
#include "fjx-fiber/internal/qspinlock.h"
#include "fjx-fiber/internal/cpu-builtin.h"

#define no_opt(x) (*(volatile typeof(x) *)(&(x)))

#define await_until(x) do { while (!(x)) { fjx_cpu_pause(); } } while (0)
#define await_until2(sx, wx) do { if (!(sx)) { await_until(wx); } } while (0)

#define relaxed_load(x) atomic_load_explicit(&(x), memory_order_relaxed)
#define relaxed_store(x, y) atomic_store_explicit(&(x), y, memory_order_relaxed)

#define acquire_load(x) atomic_load_explicit(&(x), memory_order_acquire)
#define release_store(x, y) atomic_store_explicit(&(x), y, memory_order_release)

void fjx_qspinlock_lock(fjx_qspinlock *lock) {
    fjx_mcslock mm = FJX_MCS_LOCK_INIT;

    fjx_mcslock_lock(&lock->next, &mm);

    await_until2(
        acquire_load(lock->locked) == 0,
        relaxed_load(lock->locked) == 0);
    relaxed_store(lock->locked, 1);

    fjx_mcslock_unlock(&lock->next, &mm);
}

void fjx_qspinlock_unlock(fjx_qspinlock *lock) {
    release_store(lock->locked, 0);
}
