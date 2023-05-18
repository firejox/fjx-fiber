#include "fjx-fiber/internal/timer.h"
#include "fjx-fiber/internal/scheduler.h"

typedef struct fjx_timer_node__ {
    fjx_fiber f;
    fjx_splay_node link;
    struct timespec tp;
} fjx_timer_node;

static inline bool
timespec_comp(
        struct timespec * restrict p,
        struct timespec * restrict q) {
    if (p->tv_sec != q->tv_sec) {
        return p->tv_sec < q->tv_sec;
    } else {
        return p->tv_nsec < q->tv_nsec;
    }
}

static inline bool
time_comp(
        fjx_timer_node * restrict p,
        fjx_timer_node * restrict q) {
   return timespec_comp(&p->tp, &q->tp);
}

static inline bool insert_time_node(fjx_timer_node *node, fjx_fiber_timer *timer) {
    if (fjx_splay_empty(&timer->time_tree)) {
        fjx_list_add(&timer->time_list, &node->f.link);

        fjx_splay_node_link(&node->link, NULL, &timer->time_tree.root);
        timer->min_time = timer->max_time = &node->link;
        return true;
    } else {
        fjx_timer_node *min_time = fjx_container_of(timer->min_time, fjx_timer_node, link);
        fjx_timer_node *max_time = fjx_container_of(timer->max_time, fjx_timer_node, link);

        if (time_comp(node, min_time)) {
            fjx_list_add(&timer->time_list, &node->f.link);

            fjx_splay_node_link(&node->link, &min_time->link, &min_time->link.left);
            timer->min_time = &node->link;
            fjx_splay_top(&node->link, &timer->time_tree);
            return true;
        } else if (time_comp(max_time, node)) {
            fjx_list_add_tail(&timer->time_list, &node->f.link);

            fjx_splay_node_link(&node->link, &max_time->link, &max_time->link.right);
            timer->max_time = &node->link;
            fjx_splay_top(&node->link, &timer->time_tree);
            return false;
        } else {
            fjx_splay_node **it = &timer->time_tree.root;
            fjx_splay_node *p = NULL;
            fjx_timer_node *p_node = NULL;

            for (p = *it; p != NULL; p = *it) {
                p_node = fjx_container_of(p, fjx_timer_node, link);

                if (time_comp(node, p_node)) {
                    it = &p->left;
                } else {
                    it = &p->right;
                }
            }

            p = &p_node->link;

            fjx_splay_node_link(&node->link, p, it);

            if (it == &p->left) {
                fjx_list_add_tail(&p_node->f.link, &node->f.link);
            } else {
                fjx_list_add(&p_node->f.link, &node->f.link);
            }

            fjx_splay_top(&node->link, &timer->time_tree);
            return false;
        }
    }
}

void fiber_timer_init(fjx_fiber_timer *timer) {
    timer->time_tree.root = NULL;
    timer->min_time = NULL;
    timer->max_time = NULL;
    fjx_list_init(&timer->time_list);
    fjx_thread_mutex_init(&timer->m);
    fjx_thread_cond_init(&timer->c);
}

typedef struct fjx_timer_pair__ {
    fjx_timer_node *node;
    fjx_fiber_timer *timer;
} fjx_timer_pair;

static void timer_add_fiber(void *data) {
    fjx_timer_pair *p = (fjx_timer_pair *)data;

    fjx_thread_mutex_lock(&p->timer->m);

    if (insert_time_node(p->node, p->timer)) {
        fjx_thread_cond_siganl(&p->timer->c);
    }

    fjx_thread_mutex_unlock(&p->timer->m);
}

void fiber_timer_set_timeout(
        fjx_fiber_scheduler *sched,
        fjx_fiber_timer *timer,
        struct timespec *tp) {
    fjx_timer_node node;
    struct timespec cur;

    node.tp.tv_sec = tp->tv_sec;
    node.tp.tv_nsec = tp->tv_nsec;
    fjx_splay_node_init(&node.link);
    fjx_list_init(&node.f.link);

    timespec_get(&cur, TIME_UTC);
    if (!timespec_comp(&cur, tp)) {
        fjx_thread_yield();
    } else {
        fjx_timer_pair p = {.node = &node, .timer = timer};
        get_available_fiber(sched, &node.f);
        fiber_insert_cleanup(&node.f, timer_add_fiber, &p);
        fiber_switch(&node.f);
    }
}

static void timer_thread(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    fjx_fiber_timer *timer = &sched->timer;
    struct timespec cur;
    fjx_list awake_list;
    fjx_timer_node *max_time, *min_time;

    while (true) {
        fjx_list_init(&awake_list);

        fjx_thread_mutex_lock(&timer->m);
        if (fjx_splay_empty(&timer->time_tree)) {
            do {
                fjx_thread_cond_wait(&timer->c, &timer->m);
            } while (fjx_splay_empty(&timer->time_tree));
        } else {
            while (true) {
                timespec_get(&cur, TIME_UTC);
                min_time = fjx_container_of(timer->min_time, fjx_timer_node, link);

                if (!timespec_comp(&cur, &min_time->tp)) {
                    break;
                }

                fjx_thread_cond_timedwait(&timer->c, &timer->m, &min_time->tp);
            }

            max_time = fjx_container_of(timer->max_time, fjx_timer_node, link);

            if (!timespec_comp(&cur, &max_time->tp)) {
                fjx_list_replace_init(&timer->time_list, &awake_list);
                timer->time_tree.root = NULL;
                timer->min_time = NULL;
                timer->max_time = NULL;
            } else {
                fjx_splay_node **it = &timer->time_tree.root;
                fjx_splay_node *p = NULL;
                fjx_timer_node *p_node = NULL, *up = NULL;

                for (p = *it; p != NULL; p = *it) {
                    p_node = fjx_container_of(p, fjx_timer_node, link);

                    if (timespec_comp(&cur, &p_node->tp)) {
                        up = p_node;
                        it = &p->left;
                    } else {
                        it = &p->right;
                    }
                }

                fjx_list *first = timer->time_list.next;
                fjx_list *last = up->f.link.prev;

                fjx_list_link(&awake_list, first);
                fjx_list_link(last, &awake_list);
                fjx_list_link(&timer->time_list, &up->f.link);

                fjx_splay_top(&up->link, &timer->time_tree);
                timer->min_time = &up->link;
                up->link.left = NULL;
            }
        }
        fjx_thread_mutex_unlock(&timer->m);

        enqueue_fiber_list(sched, &awake_list);
    }
}

void fiber_timer_destroy(fjx_fiber_timer *timer) {
    fjx_thread_mutex_destroy(&timer->m);
    fjx_thread_cond_destroy(&timer->c);
}

void start_timer_thread(fjx_fiber_scheduler *sched) {
    fjx_thread_spawn(timer_thread, sched);
}

enum {
    GIGA_CONSTANT = 1000000000l
};

void fiber_sleep(struct timespec *duration) {
    struct timespec cur;
    timespec_get(&cur, TIME_UTC);
    cur.tv_sec += duration->tv_sec;
    cur.tv_nsec += duration->tv_nsec;
    cur.tv_sec += cur.tv_nsec / GIGA_CONSTANT;
    cur.tv_nsec %= GIGA_CONSTANT;
    fjx_fiber_scheduler *sched = current_fiber_scheduler();
    fiber_timer_set_timeout(sched, &sched->timer, &cur);
}
