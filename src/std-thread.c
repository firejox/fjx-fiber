#include "fjx-fiber/internal/std-thread.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/cpu-builtin.h"

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

static thread_local fjx_work_thread *th = NULL;

static void work_thread_init(fjx_work_thread *t, void *stack_top) {
    t->id = thrd_current();
    t->thread_fiber.stack_top = stack_top;
    fjx_thread_semaphore_init(&t->s, 0);
}

static inline void
work_thread_loop(
        fjx_fiber_scheduler *sched,
        fjx_work_thread *thread) {
    while (true) {
        fjx_spinlock_lock(&sched->queue_lock);
        if (fjx_list_empty(&sched->fiber_list)) {
            fjx_list_add_tail(&sched->idle_thread_list, &thread->idle_link);
            fjx_spinlock_unlock(&sched->queue_lock);

            fjx_thread_semaphore_wait(&thread->s);
            fiber_switch(&thread->thread_fiber);
        } else {
            fjx_list *it = fjx_list_pop_head(&sched->fiber_list);
            fjx_spinlock_unlock(&sched->queue_lock);
            fjx_fiber *f = fjx_container_of(it, fjx_fiber, link);

            thread->thread_fiber.stack_top = f->stack_top;
            fiber_switch(&thread->thread_fiber);
        }
    }

}

static void main_work_entry(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    work_thread_loop(sched, &sched->main_th);
}

static int work_thread_entry(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    fjx_work_thread thread;
    work_thread_init(&thread, NULL);
    th = &thread;
    fjx_spinlock_lock(&sched->tlist_init_lock);
    fjx_list_add_tail(&sched->thread_list, &thread.link);
    fjx_spinlock_unlock(&sched->tlist_init_lock);
    work_thread_loop(sched, &thread);
    return 0;
}

void work_thread_spawn(fjx_fiber_scheduler *sched) {
    thrd_t id;
    int res = thrd_create(&id, work_thread_entry, sched);
    ERROR_ABORT_UNLESS(res, thrd_success, "thread create failed");
}

fjx_work_thread *current_work_thread(fjx_fiber_scheduler *sched) {
    if (th != NULL) {
        return th;
    } else {
        return &sched->main_th;
    }
}

void main_work_thread_init(fjx_fiber_scheduler *sched) {
    work_thread_init(&sched->main_th, fiber_spawn_stack_top(sched, main_work_entry, sched));
}

void work_thread_yield(void) {
    thrd_yield();
}
