#include "fjx-fiber.h"
#include <stdio.h>
#include <unistd.h>

fjx_fiber_scheduler *sched;

void f(void* data) {
    while (true) {
        puts((const char*)data);
        fiber_yield(sched);
    }
}

int main(void) {
    sched = fiber_scheduler_create(3);

    fiber_spawn(sched, f, "Hello World1");
    fiber_spawn(sched, f, "Hello World2");
    fiber_spawn(sched, f, "Hello World3");
    while (true) {
        puts("main Hello World");
        fiber_yield(sched);
    }

    return 0;
}
