#include "fjx-fiber.h"
#include <stdio.h>
#include <unistd.h>

void f(void* data) {
    while (1) {
        puts((const char*)data);
        fiber_yield();
    }
}

int main(void) {
    fiber_scheduler_init(3);

    fiber_spawn(f, "Hello World1");
    fiber_spawn(f, "Hello World2");
    fiber_spawn(f, "Hello World3");
    while (1) {
        puts("main Hello World");
        fiber_yield();
    }

    return 0;
}
