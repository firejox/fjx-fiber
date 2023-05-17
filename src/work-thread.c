#include "fjx-fiber/internal/work-thread.h"
#include "fjx-fiber/internal/scheduler.h"

static thread_local fjx_work_thread *th = NULL;

static void work_thread_init(fjx_work_thread *t, void *stack_top) {
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

static void work_thread_entry(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    fjx_work_thread thread;
    work_thread_init(&thread, NULL);

    th = &thread;

    fjx_spinlock_lock(&sched->tlist_init_lock);
    fjx_list_add_tail(&sched->thread_list, &thread.link);
    fjx_spinlock_unlock(&sched->tlist_init_lock);

    work_thread_loop(sched, &thread);
}

void work_thread_spawn(fjx_fiber_scheduler *sched) {
    fjx_thread_spawn(work_thread_entry, sched);
}

fjx_work_thread *current_work_thread(fjx_fiber_scheduler *sched) {
    if (th != NULL) {
        return th;
    } else {
        return &sched->main_th;
    }
}

void main_work_thread_init(fjx_fiber_scheduler *sched) {
    work_thread_init(
            &sched->main_th,
            fiber_spawn_stack_top(
                sched,
                main_work_entry,
                sched)
            );
}

