#include "fjx-fiber/internal/std-thread.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/cpu-builtin.h"
#include <stdlib.h>

static inline void spinlock_lock(atomic_int *lock) {
   while (atomic_exchange_explicit(lock, 1, memory_order_relaxed)) {
       while (atomic_load_explicit(lock, memory_order_relaxed)) {
           fjx_cpu_pause();
       }
   }
   atomic_thread_fence(memory_order_acquire);
}

static inline void spinlock_unlock(atomic_int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
}

void fjx_spinlock_init(fjx_spinlock *l) {
    atomic_init(&l->lock, 0);
}

void fjx_spinlock_lock(fjx_spinlock *l) {
    spinlock_lock(&l->lock);
}

void fjx_spinlock_unlock(fjx_spinlock *l) {
    spinlock_unlock(&l->lock);
}

void fjx_rwlock_init(fjx_rwlock *l) {
    mtx_init(&l->m, mtx_plain);
    cnd_init(&l->cond);
    l->w_waited = 0;
    l->r_reading = 0;
    l->on_write = 0;
}

void fjx_rwlock_lockr(fjx_rwlock *l) {
    mtx_lock(&l->m);
    while (l->w_waited > 0 || l->on_write) {
        cnd_wait(&l->cond, &l->m);
    }
    l->r_reading ++;
    mtx_unlock(&l->m);
}

void fjx_rwlock_unlockr(fjx_rwlock *l) {
    mtx_lock(&l->m);
    l->r_reading --;
    if (l->r_reading == 0) {
        cnd_broadcast(&l->cond);
    }
    mtx_unlock(&l->m);
}

void fjx_rwlock_lockw(fjx_rwlock *l) {
    mtx_lock(&l->m);
    l->w_waited ++;
    while (l->r_reading > 0 || l->on_write) {
        cnd_wait(&l->cond, &l->m);
    }
    l->w_waited --;
    l->on_write = 1;
    mtx_unlock(&l->m);
}

void fjx_rwlock_unlockw(fjx_rwlock *l) {
    mtx_lock(&l->m);
    l->on_write = 0;
    cnd_broadcast(&l->cond);
    mtx_unlock(&l->m);
}

void fjx_rwlock_destroy(fjx_rwlock *l) {
    mtx_destroy(&l->m);
    cnd_destroy(&l->cond);
}

void fjx_thread_semaphore_init(fjx_thread_semaphore *s, unsigned count) {
    mtx_init(&s->m, mtx_plain);
    cnd_init(&s->c);
    s->count = count;
}

void fjx_thread_semaphore_wait(fjx_thread_semaphore *s) {
    mtx_lock(&s->m);
    while (s->count == 0) {
        cnd_wait(&s->c, &s->m);
    }
    s->count --;
    mtx_unlock(&s->m);
}

void fjx_thread_semaphore_signal(fjx_thread_semaphore *s) {
    mtx_lock(&s->m);
    s->count ++;
    cnd_signal(&s->c);
    mtx_unlock(&s->m);
}

void fjx_thread_semaphore_destroy(fjx_thread_semaphore *s) {
    mtx_destroy(&s->m);
    cnd_destroy(&s->c);
}

void fjx_thread_mutex_init(fjx_thread_mutex *m) {
    mtx_init(&m->impl, mtx_plain);
}

void fjx_thread_mutex_lock(fjx_thread_mutex *m) {
    mtx_lock(&m->impl);
}

void fjx_thread_mutex_unlock(fjx_thread_mutex *m) {
    mtx_unlock(&m->impl);
}

void fjx_thread_mutex_destroy(fjx_thread_mutex *m) {
    mtx_destroy(&m->impl);
}

void fjx_thread_cond_init(fjx_thread_cond *c) {
    cnd_init(&c->impl);
}

void fjx_thread_cond_wait(fjx_thread_cond *c, fjx_thread_mutex *m) {
    cnd_wait(&c->impl, &m->impl);
}

void fjx_thread_cond_timedwait(
        fjx_thread_cond *c,
        fjx_thread_mutex *m,
        struct timespec *t) {
    cnd_timedwait(&c->impl, &m->impl, t);
}

void fjx_thread_cond_siganl(fjx_thread_cond *c) {
    cnd_signal(&c->impl);
}

void fjx_thread_cond_broadcast(fjx_thread_cond *c) {
    cnd_broadcast(&c->impl);
}

void fjx_thread_cond_destroy(fjx_thread_cond *c) {
    cnd_destroy(&c->impl);
}

typedef struct fjx_thread_init_pair__ {
    thread_entry entry;
    void *arg;
} fjx_thread_init_pair;

static int fjx_thread_helper(void *data) {
    fjx_thread_init_pair *p = (fjx_thread_init_pair *)data;
    thread_entry entry = p->entry;
    void *arg = p->arg;
    free(p);
    entry(arg);
    return 0;
}

void fjx_thread_spawn(thread_entry entry, void *arg) {
    fjx_thread_init_pair *p = calloc(1, sizeof(fjx_thread_init_pair));
    ERROR_ABORT_IF(p, NULL, "allocate memory failed");
    p->entry = entry;
    p->arg = arg;

    thrd_t id;
    int res = thrd_create(&id, fjx_thread_helper, p);
    ERROR_ABORT_UNLESS(res, thrd_success, "thread create failed");
}

void fjx_thread_yield(void) {
    thrd_yield();
}

