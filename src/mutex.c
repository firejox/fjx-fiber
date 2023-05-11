#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/fiber.h"
#include "fjx-fiber/mutex.h"
#include <stdlib.h>

struct fjx_fiber_mutex__ {
    fjx_spinlock lock;
    fjx_list fiber_list;
    int hold;
};

fjx_fiber_mutex *fiber_mutex_create(void) {
    fjx_fiber_mutex *m = calloc(1, sizeof(fjx_fiber_mutex));

    ERROR_ABORT_IF(m, NULL, "allocate fiber mutex failed");

    fjx_spinlock_init(&m->lock);
    fjx_list_init(&m->fiber_list);
    m->hold = 0;

    return m;
}

void fiber_mutex_lock(
        fjx_fiber_scheduler *sched,
        fjx_fiber_mutex *m) {

    fjx_spinlock_lock(&m->lock);
    if (!m->hold) {
        m->hold = 1;
        fjx_spinlock_unlock(&m->lock);
    } else {
        fjx_fiber f;

        fjx_list_add_tail(&m->fiber_list, &f.link);

        fjx_spinlock_lock(&sched->queue_lock);
        f.stack_top = get_available_fiber_unsafe(sched)->stack_top;
        fjx_spinlock_unlock(&sched->queue_lock);

        fiber_insert_cleanup(&f, (cleanup_func_t)fjx_spinlock_unlock, &m->lock);
        fiber_switch(&f);
    }
}

void fiber_mutex_unlock(
        fjx_fiber_scheduler *sched,
        fjx_fiber_mutex *m) {
    fjx_spinlock_lock(&m->lock);
    if (fjx_list_empty(&m->fiber_list)) {
        m->hold = 0;
        fjx_spinlock_unlock(&m->lock);
    } else {
        fjx_list *it = fjx_list_pop_head(&m->fiber_list);
        fjx_spinlock_unlock(&m->lock);
        fjx_fiber *f = fjx_container_of(it, fjx_fiber, link);
        enqueue_fiber(sched, f);
    }
}

void fiber_mutex_destroy(
        fjx_fiber_scheduler *sched,
        fjx_fiber_mutex *m) {
    fjx_list fiber_list;
    fjx_list_init(&fiber_list);

    fjx_spinlock_lock(&m->lock);
    if (m->hold == 1) {
        if (!fjx_list_empty(&m->fiber_list)) {
            fjx_list_replace_init(&m->fiber_list, &fiber_list);
        }
    }
    fjx_spinlock_unlock(&m->lock);
    fjx_spinlock_destroy(&m->lock);
    free(m);

    enqueue_fiber_list(sched, &fiber_list);
}
