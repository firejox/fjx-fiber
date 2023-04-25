#include "fjx-fiber/internal/mcsh-lock.h"
#include "fjx-fiber/internal/cpu-builtin.h"

struct mcsh_qnode__;
typedef struct mcsh_qnode__ mcsh_qnode;
typedef _Atomic(mcsh_qnode *) atomic_mcsh_qnode_ptr;

struct mcsh_qnode__ {
    atomic_mcsh_qnode_ptr next;
    int own_lock;
};

#define no_opt(x) (*(volatile __typeof__(x) *)(&(x)))

#define await_until(x) while(!(x)) { fjx_cpu_pause(); }

void fjx_mcshlock_lock(fjx_mcshlock *lock) {
    mcsh_qnode mm = {NULL, 0};

    mcsh_qnode *prev = (mcsh_qnode *)atomic_exchange_explicit(
            &lock->tail,
            &mm,
            memory_order_relaxed);

    if (prev == NULL) {
        await_until(!lock->holding);
        lock->holding = 1;
    } else {
        atomic_store_explicit(&prev->next, &mm, memory_order_release);
        await_until(no_opt(mm.own_lock));
    }

    mcsh_qnode *succ = atomic_load_explicit(&mm.next, memory_order_relaxed);

    if (succ == NULL) {
        void *tmp = &mm;
        if (!atomic_compare_exchange_strong_explicit(
                    &lock->tail,
                    &tmp,
                    NULL,
                    memory_order_release,
                    memory_order_relaxed)) {
            await_until((succ = atomic_load_explicit(
                    &mm.next,
                    memory_order_relaxed)) != NULL);
        }
    }

    atomic_thread_fence(memory_order_acquire);
    lock->msg = succ;
}

void fjx_mcshlock_unlock(fjx_mcshlock *lock) {
    mcsh_qnode *succ = (mcsh_qnode *)lock->msg;
    if (succ != NULL) {
        succ->own_lock = 1;
    } else {
        lock->holding = 0;
    }
}
