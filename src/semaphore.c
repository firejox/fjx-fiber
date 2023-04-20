#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/fiber.h"
#include "fjx-fiber/semaphore.h"
#include <stdlib.h>

struct __fjx_fiber_semaphore {
    fjx_spinlock lock;
    fjx_list fiber_list;
    unsigned int count;
};

fjx_fiber_semaphore *fiber_semaphore_create(unsigned int count) {
    fjx_fiber_semaphore *s = calloc(1, sizeof(fjx_fiber_semaphore));

    ERROR_ABORT_IF(s, NULL, "allocate fiber semaphore failed");

    fjx_spinlock_init(&s->lock);
    fjx_list_init(&s->fiber_list);
    s->count = count;

    return s;
}

void fiber_semaphore_signal(
        fjx_fiber_scheduler *sched,
        fjx_fiber_semaphore *s) {
    fjx_spinlock_lock(&s->lock);
    if (!fjx_list_empty(&s->fiber_list)) {
        fjx_list *it = fjx_list_pop_head(&s->fiber_list);
        fjx_spinlock_unlock(&s->lock);
        fjx_fiber *f = fjx_container_of(it, fjx_fiber, link);
        enqueue_fiber(sched, f);
    } else {
        s->count ++;
        fjx_spinlock_unlock(&s->lock);
    }
}

void fiber_semaphore_wait(
        fjx_fiber_scheduler *sched,
        fjx_fiber_semaphore *s) {
    fjx_spinlock_lock(&s->lock);
    if (s->count > 0) {
        s->count --;
        fjx_spinlock_unlock(&s->lock);
    } else {
        fjx_fiber f;
        fjx_list_add_tail(&s->fiber_list, &f.link);

        fjx_spinlock_lock(&sched->queue_lock);
        f.stack_top = get_avaiable_fiber_unsafe(sched)->stack_top;
        fjx_spinlock_unlock(&sched->queue_lock);

        fiber_insert_cleanup(&f, (cleanup_func_t)fjx_spinlock_unlock, &s->lock);
        fiber_switch(&f);
    }
}

void fiber_semaphore_destroy(
        fjx_fiber_scheduler *sched,
        fjx_fiber_semaphore *s) {
    fjx_list fiber_list;
    fjx_list_init(&fiber_list);

    fjx_spinlock_lock(&s->lock);
    if (s->count == 0) {
        if (!fjx_list_empty(&s->fiber_list)) {
            fjx_list_replace_init(&s->fiber_list, &fiber_list);
        }
    }
    fjx_spinlock_unlock(&s->lock);
    fjx_spinlock_destroy(&s->lock);
    free(s);

    enqueue_fiber_list(sched, &fiber_list);
}
