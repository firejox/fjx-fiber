#include "fjx-fiber/internal/mcs-lock.h"
#include "fjx-fiber/internal/cpu-builtin.h"

#define no_opt(x) (*(volatile typeof(x) *)(&(x)))

void fjx_mcslock_init(fjx_mcslock_shared *lock) {
    atomic_init(lock, NULL);
}

void fjx_mcslock_lock(fjx_mcslock_shared *lock, fjx_mcslock *node) {
    fjx_mcslock *prev;

    node->next = NULL;
    node->locked = 0;
    prev = atomic_exchange_explicit(lock, node, memory_order_relaxed);
    atomic_thread_fence(memory_order_acquire);

    if (prev == NULL) {
        return;
    }

    prev->next = node;
    while (no_opt(node->locked) == 0) {
        fjx_cpu_pause();
    }
}

void fjx_mcslock_unlock(fjx_mcslock_shared *lock, fjx_mcslock *node) {
    fjx_mcslock *tmp = node;
    fjx_mcslock *next = node->next;

    if (next == NULL) {
        if (atomic_compare_exchange_strong_explicit(
                    lock,
                    &tmp,
                    NULL,
                    memory_order_release,
                    memory_order_relaxed)) {
            return;
        }

        while ((next = no_opt(node->next)) == NULL) {
            fjx_cpu_pause();
        }
    }

    next->locked = 1;
}
