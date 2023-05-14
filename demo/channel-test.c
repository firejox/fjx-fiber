#include "fjx-fiber.h"
#include <stdio.h>

fjx_fiber_channel *ch;

void f(void *data) {
    int value = 0;

    fiber_channel_receive(ch, &value);
    printf("fiber receive %d\n", value);
}

int main(void) {
    fiber_scheduler_init(2);
    ch = fiber_channel_create(sizeof(int));

    fiber_spawn(f, NULL);

    int value = 10;
    printf("fiber send %d\n", value);
    fiber_channel_send(ch, &value);
    fiber_channel_destroy(ch);
    return 0;
}
