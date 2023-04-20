#include "fjx-fiber.h"
#include <stdio.h>

fjx_fiber_channel *ch;

void f(void *data) {
    fjx_fiber_scheduler *sched = (fjx_fiber_scheduler *)data;
    int value = 0;

    fiber_channel_receive(sched, ch, &value);
    printf("fiber receive %d\n", value);
}

int main(void) {
    fjx_fiber_scheduler *sched = NULL;
    sched = fiber_scheduler_create(2);
    ch = fiber_channel_create(sizeof(int));

    fiber_spawn(sched, f, sched);

    int value = 10;
    printf("fiber send %d\n", value);
    fiber_channel_send(sched, ch, &value);
    fiber_channel_destroy(sched, ch);
    return 0;
}
