#include <stdatomic.h>
#include "fjx-fiber/internal/mcsh-lock.h"
#include "fjx-fiber/internal/cpu-builtin.h"

#define no_opt(x) (*(volatile __typeof__(x) *)(&(x)))

#define await_until(x) while(!(x)) { fjx_cpu_pause(); }

void fjx_mcshlock_lock(fjx_mcshlock *lock) {
    fjx_mcshlock mm = {lock, NULL};

    fjx_mcshlock *prev = atomic_exchange_explicit(
            &lock->tail,
            &mm,
            memory_order_relaxed);

    if (prev == NULL) {
        await_until(lock->msg == NULL);
        lock->msg = lock;
    } else {
        atomic_store_explicit(&prev->tail, &mm, memory_order_release);
        await_until(no_opt(mm.msg) != NULL);
    }

    fjx_mcshlock *succ = atomic_load_explicit(&mm.tail, memory_order_acquire);

    if (succ == lock) {
        fjx_mcshlock *tmp = &mm;
        if (!atomic_compare_exchange_strong_explicit(
                    &lock->tail,
                    &tmp,
                    NULL,
                    memory_order_release,
                    memory_order_relaxed)) {
            await_until((succ = atomic_load_explicit(
                    &mm.tail,
                    memory_order_relaxed)) != lock);
        }
    }

    lock->msg = succ;
}

void fjx_mcshlock_unlock(fjx_mcshlock *lock) {
    fjx_mcshlock *succ = lock->msg;
    if (succ != lock) {
        succ->msg = lock;
    } else {
        lock->msg = NULL;
    }
}
