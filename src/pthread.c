#define USE_SYSTEM_THREAD
#define _DEFAULT_SOURCE
#include "fjx-fiber/internal/pthread.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/debug.h"

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
