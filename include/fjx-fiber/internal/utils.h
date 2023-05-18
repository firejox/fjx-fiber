#ifndef fjx_fiber_internal_utils_h
#define fjx_fiber_internal_utils_h

#include "./visibility.h"
#include "fjx-utils/list.h"
#include "fjx-utils/avl_tree.h"
#include "fjx-utils/splay_tree.h"

struct fjx_fiber__;
typedef struct fjx_fiber__ fjx_fiber;

struct fjx_fiber_scheduler__;
typedef struct fjx_fiber_scheduler__ fjx_fiber_scheduler;

struct fjx_fiber_memory__;
typedef struct fjx_fiber_memory__ fjx_fiber_memory;

struct fjx_work_thread__;
typedef struct fjx_work_thread__ fjx_work_thread;

struct fjx_fiber_timer__;
typedef struct fjx_fiber_timer__ fjx_fiber_timer;

#endif
