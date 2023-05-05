#include <stdatomic.h>
#include "fjx-fiber/internal/mcs-lock.h"
#include "fjx-fiber/internal/cpu-builtin.h"

#define no_opt(x) (*(volatile typeof(x) *)(&(x)))

#define await_until(x) do { while (!(x)) { fjx_cpu_pause(); } } while (0)
#define await_until2(sx, wx) do { if (!(sx)) { await_until(wx); } } while (0)

#define relaxed_load(x) atomic_load_explicit(&(x), memory_order_relaxed)
#define relaxed_store(x, y) atomic_store_explicit(&(x), y, memory_order_relaxed)

#define acquire_load(x) atomic_load_explicit(&(x), memory_order_acquire)
#define release_store(x, y) atomic_store_explicit(&(x), y, memory_order_release)

void fjx_mcslock_lock(fjx_mcslock_atomic_ptr *lock, fjx_mcslock *node) {
    fjx_mcslock *prev = atomic_exchange_explicit(lock, node, memory_order_relaxed);

    if (prev == NULL) {
        return;
    }

    release_store(prev->next, node);
    await_until2(
        acquire_load(node->locked) != 0,
        relaxed_load(node->locked) != 0);
}

void fjx_mcslock_unlock(fjx_mcslock_atomic_ptr *lock, fjx_mcslock *node) {
    fjx_mcslock *tmp = node;
    fjx_mcslock *next = acquire_load(node->next);

    if (next == NULL) {
        if (atomic_compare_exchange_strong_explicit(
                    lock,
                    &tmp,
                    NULL,
                    memory_order_release,
                    memory_order_relaxed)) {
            return;
        }

        await_until((next = relaxed_load(node->next)) != NULL);
    }

    release_store(next->locked, 1);
}
