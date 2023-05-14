#include "fjx-fiber.h"
#include <stdio.h>

void f(void *data) {
    puts("fiber Hello World");
}

int main(void) {
    fiber_scheduler_init(0);

    fiber_spawn(f, NULL);
    fiber_yield();

    puts("main Hello World");

    return 0;
}
