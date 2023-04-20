#include "fjx-fiber/internal/debug.h"
#include "fjx-fiber/internal/scheduler.h"
#include "fjx-fiber/internal/fiber.h"
#include "fjx-fiber/internal/spinlock.h"
#include "fjx-fiber/channel.h"
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#define CHANNEL_ITEM_INIT -1

typedef struct {
    void *data;
    fjx_fiber fiber;
    fjx_list link;
    int state;
} channel_item;

struct fjx_fiber_channel__ {
    fjx_spinlock lock;
    fjx_list ritem_list;
    fjx_list witem_list;
    size_t item_size;
    bool closed;
};

fjx_fiber_channel *fiber_channel_create(size_t item_size) {
    fjx_fiber_channel *ch = calloc(1, sizeof(fjx_fiber_channel));
    ERROR_ABORT_IF(ch, NULL, "allocate ch failed");

    fjx_spinlock_init(&ch->lock);
    fjx_list_init(&ch->ritem_list);
    fjx_list_init(&ch->witem_list);
    ch->item_size = item_size;
    ch->closed = false;
    return ch;
}

int fiber_channel_send(
        fjx_fiber_scheduler *sched,
        fjx_fiber_channel *ch,
        void* data) {
    channel_item src;
    src.state = CHANNEL_ITEM_INIT;
    src.data = data;

    if (!ch->closed) {
        fjx_spinlock_lock(&ch->lock);
        if (!ch->closed) {
            if (!fjx_list_empty(&ch->ritem_list)) {
                fjx_list *it = fjx_list_pop_head(&ch->ritem_list);
                fjx_spinlock_unlock(&ch->lock);

                channel_item *dest = fjx_container_of(it, channel_item, link);
                memcpy(dest->data, src.data, ch->item_size);
                dest->state = CHANNEL_SUCCESS;
                src.fiber.stack_top = dest->fiber.stack_top;

                fjx_spinlock_lock(&sched->queue_lock);
                fjx_list_add_tail(&sched->fiber_list, &src.fiber.link);
                fiber_insert_cleanup(&src.fiber, (cleanup_func_t)fjx_spinlock_unlock, &sched->queue_lock);
                fiber_switch(&src.fiber);
                return CHANNEL_SUCCESS;
            } else {
                fjx_list_add_tail(&ch->witem_list, &src.link);

                fjx_spinlock_lock(&sched->queue_lock);
                src.fiber.stack_top = get_avaiable_fiber_unsafe(sched)->stack_top;
                fjx_spinlock_unlock(&sched->queue_lock);
                fiber_insert_cleanup(&src.fiber, (cleanup_func_t)fjx_spinlock_unlock, &ch->lock);
                fiber_switch(&src.fiber);
                return src.state;
            }
        } else {
            fjx_spinlock_unlock(&ch->lock);
            return CHANNEL_CLOSED;
        }
    } else {
        return CHANNEL_CLOSED;
    }
}

int fiber_channel_receive(
        fjx_fiber_scheduler *sched,
        fjx_fiber_channel *ch,
        void *data) {
    channel_item dest;
    dest.state = CHANNEL_ITEM_INIT;
    dest.data = data;

    fjx_spinlock_lock(&ch->lock);
    if (fjx_list_empty(&ch->witem_list)) {
        if (!ch->closed) {
            fjx_list_add_tail(&ch->ritem_list, &dest.link);

            fjx_spinlock_lock(&sched->queue_lock);
            dest.fiber.stack_top = get_avaiable_fiber_unsafe(sched)->stack_top;
            fjx_spinlock_unlock(&sched->queue_lock);
            fiber_insert_cleanup(&dest.fiber, (cleanup_func_t)fjx_spinlock_unlock, &ch->lock);
            fiber_switch(&dest.fiber);
            return dest.state;
        } else {
            fjx_spinlock_unlock(&ch->lock);
            return CHANNEL_CLOSED;
        }
    } else {
        fjx_list *it = fjx_list_pop_head(&ch->witem_list);
        fjx_spinlock_unlock(&ch->lock);

        channel_item *src = fjx_container_of(it, channel_item, link);
        memcpy(dest.data, src->data, ch->item_size);
        src->state = CHANNEL_SUCCESS;
        dest.fiber.stack_top = src->fiber.stack_top;

        fjx_spinlock_lock(&sched->queue_lock);
        fjx_list_add_tail(&sched->fiber_list, &dest.fiber.link);
        fiber_insert_cleanup(&dest.fiber, (cleanup_func_t)fjx_spinlock_unlock, &sched->queue_lock);
        fiber_switch(&dest.fiber);
        return CHANNEL_SUCCESS;
    }
}

void fiber_channel_close(
        fjx_fiber_scheduler *sched,
        fjx_fiber_channel *ch) {
    fjx_list ritem_list, fiber_list, *it = NULL;
    if (!ch->closed) {
        fjx_list_init(&ritem_list);
        fjx_spinlock_lock(&ch->lock);
        if (!ch->closed) {
            if (!fjx_list_empty(&ch->ritem_list)) {
                fjx_list_replace_init(&ch->ritem_list, &ritem_list);
            }
            ch->closed = true;
            fjx_spinlock_unlock(&ch->lock);

            fjx_list_init(&fiber_list);
            fjx_list_foreach(it, &ritem_list) {
                channel_item *item = fjx_container_of(it, channel_item, link);
                item->state = CHANNEL_CLOSED;
                fjx_list_add_tail(&fiber_list, &item->fiber.link);
            }
            enqueue_fiber_list(sched, &fiber_list);
        } else {
            fjx_spinlock_unlock(&ch->lock);
        }
    }
}

void fiber_channel_destroy(
        fjx_fiber_scheduler *sched,
        fjx_fiber_channel *ch) {
    fjx_list ritem_list, witem_list, fiber_list, *it = NULL;
    fjx_list_init(&ritem_list);
    fjx_list_init(&witem_list);

    fjx_spinlock_lock(&ch->lock);
    ch->closed = true;
    if (!fjx_list_empty(&ch->ritem_list)) {
        fjx_list_replace_init(&ch->ritem_list, &ritem_list);
    } else if (!fjx_list_empty(&ch->witem_list)) {
        fjx_list_replace_init(&ch->witem_list, &witem_list);
    }
    fjx_spinlock_unlock(&ch->lock);

    fjx_spinlock_destroy(&ch->lock);
    free(ch);

    fjx_list_init(&fiber_list);
    fjx_list_foreach(it, &ritem_list) {
        channel_item *item = fjx_container_of(it, channel_item, link);
        item->state = CHANNEL_DESTROY;
        fjx_list_add_tail(&fiber_list, &item->fiber.link);
    }

    fjx_list_foreach(it, &witem_list) {
        channel_item *item = fjx_container_of(it, channel_item, link);
        item->state = CHANNEL_DESTROY;
        fjx_list_add_tail(&fiber_list, &item->fiber.link);
    }

    enqueue_fiber_list(sched, &fiber_list);
}
