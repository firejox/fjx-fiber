#ifndef fjx_fiber_internal_timer_h
#define fjx_fiber_internal_timer_h

#include "../timer.h"
#include "./utils.h"
#include "./thread-cond.h"
#include "./thread-mutex.h"

struct fjx_fiber_timer__ {
    fjx_splay time_tree;
    fjx_splay_node *min_time;
    fjx_splay_node *max_time;

    fjx_list time_list;

    fjx_thread_mutex m;
    fjx_thread_cond c;
};

DLL_LOCAL void fiber_timer_init(fjx_fiber_timer *);
DLL_LOCAL void fiber_timer_set_timeout(
        fjx_fiber_scheduler *,
        fjx_fiber_timer *,
        struct timespec *);
DLL_LOCAL void fiber_timer_destroy(fjx_fiber_timer *);
DLL_LOCAL void start_timer_thread(fjx_fiber_scheduler *);

#endif
