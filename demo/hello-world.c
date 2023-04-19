#include "fjx-fiber.h"
#include <stdio.h>

void f(void *data) {
    puts("fiber Hello World");
}

int main(void) {
    fjx_fiber_scheduler *sched;
    sched = fiber_scheduler_create(0);

    fiber_spawn(sched, f, NULL);
    fiber_yield(sched);

    puts("main Hello World");

    return 0;
}
