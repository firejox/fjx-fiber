#define USE_SYSTEM_THREAD
#define _DEFAULT_SOURCE
#include "fjx-fiber/internal/pthread.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/debug.h"
#include <stdlib.h>
#include <sched.h>

void fjx_spinlock_init(fjx_spinlock *lock) {
    int res = pthread_spin_init(&lock->impl, PTHREAD_PROCESS_PRIVATE);
    ERROR_ABORT_UNLESS(res, 0, "spinlock init failed");
}

void fjx_spinlock_lock(fjx_spinlock *lock) {
    int res = pthread_spin_lock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "spinlock lock failed");
}

void fjx_spinlock_unlock(fjx_spinlock *lock) {
    int res = pthread_spin_unlock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "spinlock unlock failed");
}

void fjx_spinlock_destroy(fjx_spinlock *lock) {
    int res = pthread_spin_destroy(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "spinlock destroy failed");
}

void fjx_rwlock_init(fjx_rwlock *lock) {
    int res = pthread_rwlock_init(&lock->impl, NULL);
    ERROR_ABORT_UNLESS(res, 0, "rwlock init failed");
}

void fjx_rwlock_lockr(fjx_rwlock *lock) {
    int res = pthread_rwlock_rdlock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "rwlock read lock failed");
}

void fjx_rwlock_unlockr(fjx_rwlock *lock) {
    int res = pthread_rwlock_unlock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "rwlock read unlock failed");
}

void fjx_rwlock_lockw(fjx_rwlock *lock) {
    int res = pthread_rwlock_wrlock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "rwlock write lock failed");
}

void fjx_rwlock_unlockw(fjx_rwlock *lock) {
    int res = pthread_rwlock_unlock(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "rwlock write unlock failed");
}

void fjx_rwlock_destroy(fjx_rwlock *lock) {
    int res = pthread_rwlock_destroy(&lock->impl);
    ERROR_ABORT_UNLESS(res, 0, "rwlock destroy failed");
}

void fjx_thread_semaphore_init(fjx_thread_semaphore *s, unsigned count) {
    int res = sem_init(&s->impl, 0, count);
    ERROR_ABORT_UNLESS(res, 0, "thread semaphore init failed");
}

void fjx_thread_semaphore_wait(fjx_thread_semaphore *s) {
    sem_wait(&s->impl);
}

void fjx_thread_semaphore_signal(fjx_thread_semaphore *s) {
    sem_post(&s->impl);
}

void fjx_thread_semaphore_destroy(fjx_thread_semaphore *s) {
    int res = sem_destroy(&s->impl);
    ERROR_ABORT_UNLESS(res, 0, "thread semaphore destroy failed");
}

void fjx_thread_mutex_init(fjx_thread_mutex *m) {
    int res = pthread_mutex_init(&m->impl, NULL);
    ERROR_ABORT_UNLESS(res, 0, "thread mutex init failed");
}

void fjx_thread_mutex_lock(fjx_thread_mutex *m) {
    pthread_mutex_lock(&m->impl);
}

void fjx_thread_mutex_unlock(fjx_thread_mutex *m) {
    pthread_mutex_unlock(&m->impl);
}

void fjx_thread_mutex_destroy(fjx_thread_mutex *m) {
    int res = pthread_mutex_destroy(&m->impl);
    ERROR_ABORT_UNLESS(res, 0, "thread mutex destroy failed");
}

void fjx_thread_cond_init(fjx_thread_cond *c) {
    int res = pthread_cond_init(&c->impl, NULL);
    ERROR_ABORT_UNLESS(res, 0, "thread condition init failed");
}

void fjx_thread_cond_wait(fjx_thread_cond *c, fjx_thread_mutex *m) {
    pthread_cond_wait(&c->impl, &m->impl);
}

void fjx_thread_cond_timedwait(
        fjx_thread_cond *c,
        fjx_thread_mutex *m,
        struct timespec *t) {
    pthread_cond_timedwait(&c->impl, &m->impl, t);
}

void fjx_thread_cond_siganl(fjx_thread_cond *c) {
    pthread_cond_signal(&c->impl);
}

void fjx_thread_cond_broadcast(fjx_thread_cond *c) {
    pthread_cond_broadcast(&c->impl);
}

void fjx_thread_cond_destroy(fjx_thread_cond *c) {
    int res = pthread_cond_destroy(&c->impl);
    ERROR_ABORT_UNLESS(res, 0, "thread condition destroy failed");
}

typedef struct fjx_thread_init_pair__ {
    thread_entry entry;
    void *arg;
} fjx_thread_init_pair;

static void *fjx_thread_helper(void *data) {
    fjx_thread_init_pair *p = (fjx_thread_init_pair *)data;
    thread_entry entry = p->entry;
    void *arg = p->arg;
    free(p);
    entry(arg);
    return NULL;
}

void fjx_thread_spawn(thread_entry entry, void *arg) {
    fjx_thread_init_pair *p = calloc(1, sizeof(fjx_thread_init_pair));
    ERROR_ABORT_IF(p, NULL, "allocate memory failed");
    p->entry = entry;
    p->arg = arg;

    pthread_t id;
    int res = pthread_create(&id, NULL, fjx_thread_helper, p);
    ERROR_ABORT_UNLESS(res, 0, "thread create failed");
}

void fjx_thread_yield(void) {
    sched_yield();
}

static _Thread_local fjx_work_thread *th = NULL;

static void work_thread_init(fjx_work_thread *t, void *stack_top) {
    assert(th == NULL);

    t->id = pthread_self();
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
            fjx_list *it = sched->fiber_list.next;
            fjx_list_unlink(it);
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

static void *work_thread_entry(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    fjx_work_thread thread;
    work_thread_init(&thread, NULL);
    th = &thread;
    fjx_spinlock_lock(&sched->tlist_init_lock);
    fjx_list_add_tail(&sched->thread_list, &thread.link);
    fjx_spinlock_unlock(&sched->tlist_init_lock);
    work_thread_loop(sched, &thread);
    return NULL;
}

void work_thread_spawn(fjx_fiber_scheduler *sched) {
    pthread_t id;
    int res = pthread_create(&id, NULL, work_thread_entry, sched);
    ERROR_ABORT_UNLESS(res, 0, "thread create failed");
}

fjx_work_thread *current_work_thread(fjx_fiber_scheduler *sched) {
    if (th != NULL) {
        return th;
    } else {
        return &sched->main_th;
    }
}

void work_thread_awake(fjx_work_thread *th, fjx_fiber *f) {
    th->thread_fiber.stack_top = f->stack_top;
    fjx_thread_semaphore_signal(&th->s);
}

void main_work_thread_init(fjx_fiber_scheduler *sched) {
    work_thread_init(&sched->main_th, fiber_spawn_stack_top(sched, main_work_entry, sched));
}

void work_thread_yield(void) {
    sched_yield();
}
