#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/fiber.h"
#include "fjx-fiber/internal/fiber-memory.h"
#include <stdlib.h>

static void
fiber_scheduler_init_impl(
        fjx_fiber_scheduler *sched,
        int num_threads) {
    if (num_threads < 0) {
        num_threads = 0;
    }

    fjx_spinlock_init(&sched->queue_lock);
    fjx_list_init(&sched->fiber_list);
    fjx_list_init(&sched->idle_thread_list);

    fjx_spinlock_init(&sched->tlist_init_lock);
    fjx_list_init(&sched->thread_list);

    fjx_spinlock_init(&sched->disposed_fiber_lock);
    fjx_list_init(&sched->disposed_fiber_list);

    fjx_rwlock_init(&sched->pool_lock);
    sched->fiber_pool.root = NULL;

    main_work_thread_init(sched);
    fjx_list_add_tail(&sched->thread_list, &sched->main_th.link);

    for (int i = 0; i < num_threads; i++) {
        work_thread_spawn(sched);
    }
}

static fjx_fiber_scheduler main_sched, *cur_sched = NULL;

void fiber_scheduler_init(int num_threads) {
    if (cur_sched == NULL) {
        fiber_scheduler_init_impl(&main_sched, num_threads);
        cur_sched = &main_sched;
    }
}

fjx_fiber_scheduler *current_fiber_scheduler(void) {
    return cur_sched;
}

bool get_available_fiber(
        fjx_fiber_scheduler *sched,
        fjx_fiber *f) {
    fjx_spinlock_lock(&sched->queue_lock);
    if (fjx_list_empty(&sched->fiber_list)) {
        fjx_spinlock_unlock(&sched->queue_lock);
        f->stack_top = current_work_thread(sched)->thread_fiber.stack_top;
        return true;
    } else {
        fjx_list *it = fjx_list_pop_head(&sched->fiber_list);
        fjx_spinlock_unlock(&sched->queue_lock);
        f->stack_top = fjx_container_of(it, fjx_fiber, link)->stack_top;
        return false;
    }
}

static inline bool
fiber_memory_in_range(
        fjx_fiber_memory *mem,
        void * const addr) {
    return (mem->addr <= addr) && (addr < mem->stack_top);
}

static inline fjx_fiber_memory *
query_memory(
        fjx_fiber_scheduler *sched,
        void *addr) {
    fjx_rwlock_lockr(&sched->pool_lock);

    fjx_avl_node *it = sched->fiber_pool.root;

    while (it != NULL) {
        fjx_fiber_memory *m = fjx_container_of(it, fjx_fiber_memory, link);

        if (fiber_memory_in_range(m, addr)) {
            fjx_rwlock_unlockr(&sched->pool_lock);
            return m;
        } else if (m->addr < addr) {
            it = it->right;
        } else {
            it = it->left;
        }
    }

    fjx_rwlock_unlockr(&sched->pool_lock);
    return NULL;
}

static inline void insert_memory(
        fjx_fiber_scheduler *sched,
        fjx_fiber_memory *mem) {
    fjx_rwlock_lockw(&sched->pool_lock);
    fjx_avl_node **it = &sched->fiber_pool.root, *pa = NULL;

    while (*it != NULL) {
        fjx_fiber_memory *m = fjx_container_of(*it, fjx_fiber_memory, link);

        pa = *it;
        if (m->addr < mem->addr) {
            it = &pa->right;
        } else {
            it = &pa->left;
        }
    }

    fjx_avl_node_link(&mem->link, pa, it);
    fjx_avl_insert_bf(&mem->link, &sched->fiber_pool);
    fjx_rwlock_unlockw(&sched->pool_lock);
}

fjx_fiber_memory *get_available_memory(
        fjx_fiber_scheduler *sched) {
    fjx_spinlock_lock(&sched->disposed_fiber_lock);

    if (fjx_list_empty(&sched->disposed_fiber_list)) {
        fjx_spinlock_unlock(&sched->disposed_fiber_lock);

        fjx_fiber_memory *mem = fiber_memory_create();
        insert_memory(sched, mem);
        return mem;
    } else {
        fjx_list *it = fjx_list_pop_head(&sched->disposed_fiber_list);
        fjx_spinlock_unlock(&sched->disposed_fiber_lock);

        fjx_fiber *f = fjx_container_of(it, fjx_fiber, link);
        return query_memory(sched, f->stack_top);
    }
}

fjx_work_thread *try_get_idle_thread_unsafe(
        fjx_fiber_scheduler *sched) {
    if (!fjx_list_empty(&sched->idle_thread_list)) {
        fjx_list *it = fjx_list_pop_head(&sched->idle_thread_list);

        return fjx_container_of(it, fjx_work_thread, idle_link);
    } else {
        return NULL;
    }
}

void enqueue_fiber(
        fjx_fiber_scheduler *sched,
        fjx_fiber *f) {
    fjx_work_thread *th = NULL;

    fjx_spinlock_lock(&sched->queue_lock);
    if ((th = try_get_idle_thread_unsafe(sched)) != NULL) {
        fjx_spinlock_unlock(&sched->queue_lock);

        th->thread_fiber.stack_top = f->stack_top;
        fjx_thread_semaphore_signal(&th->s);
    } else {
        fjx_list_add_tail(&sched->fiber_list, &f->link);
        fjx_spinlock_unlock(&sched->queue_lock);
    }
}

void enqueue_fiber_pair(
        fjx_fiber_pair *pair) {
    enqueue_fiber(pair->sched, pair->f);
}

void enqueue_fiber_list(
        fjx_fiber_scheduler *sched,
        fjx_list *fiber_list) {
    fjx_list *ix = NULL, *iy = NULL;
    fjx_list thread_list;
    fjx_list_init(&thread_list);

    while (fjx_list_empty(fiber_list)) {
        fjx_spinlock_lock(&sched->queue_lock);
        if (fjx_list_empty(&sched->idle_thread_list)) {
            fjx_list_add_list_tail(&sched->fiber_list, fiber_list);
            fjx_spinlock_unlock(&sched->queue_lock);
            return;
        } else {
            fjx_list_replace_init(&sched->idle_thread_list, &thread_list);
            fjx_spinlock_unlock(&sched->queue_lock);

            while (true) {
                for (ix = fiber_list->next, iy = thread_list.next;
                        ix != fiber_list && iy != &thread_list;
                        ix = ix->next, iy = iy->next) {
                    fjx_fiber *f = fjx_container_of(ix, fjx_fiber, link);
                    fjx_work_thread *th = fjx_container_of(iy, fjx_work_thread, idle_link);

                    th->thread_fiber.stack_top = f->stack_top;
                    fjx_thread_semaphore_signal(&th->s);
                }

                if (ix == fiber_list) {
                    thread_list.next = iy;

                    if (iy == &thread_list) {
                        return;
                    } else {
                        fjx_spinlock_lock(&sched->queue_lock);
                        if (fjx_list_empty(&sched->fiber_list)) {
                            fjx_list_add_list(&sched->idle_thread_list, &thread_list);
                            fjx_spinlock_unlock(&sched->queue_lock);
                            return;
                        } else {
                            fjx_list_replace_init(&sched->fiber_list, fiber_list);
                            fjx_spinlock_unlock(&sched->queue_lock);
                        }
                    }
                } else {
                    fiber_list->next = ix;
                    break;
                }
            }
        }
    }
}

void recycle_fiber_pair(fjx_fiber_pair *pair) {
    fjx_spinlock_lock(&pair->sched->disposed_fiber_lock);
    fjx_list_add_tail(&pair->sched->disposed_fiber_list, &pair->f->link);
    fjx_spinlock_unlock(&pair->sched->disposed_fiber_lock);
}
