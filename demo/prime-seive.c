#include "fjx-fiber.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct gen_data {
    fjx_fiber_channel *ch;
};

struct filter_data {
    fjx_fiber_channel *ch;
    fjx_fiber_channel *ch_next;
    int prime;
};

void generate(void *data) {
    struct gen_data *d = (struct gen_data*)data;

    for (int i = 2; ; i++) {
        fiber_channel_send(d->ch, &i);
    }
}

void filter(void *data) {
    struct filter_data *d = (struct filter_data*)data;
    int i;
    while (true) {
        fiber_channel_receive(d->ch, &i);
        if (i % d->prime) {
            fiber_channel_send(d->ch_next, &i);
        }
    }
}

int main(void) {
    fjx_fiber_channel *ch = NULL, *ch_next = NULL;

    fiber_scheduler_init(7);
    ch = fiber_channel_create(sizeof(int));
    struct gen_data g = {.ch = ch};
    struct filter_data f[103];

    fiber_spawn(generate, &g);

    for (int i = 0; i < 100; i++) {
        int prime;
        fiber_channel_receive(ch, &prime);
        printf("%d\n", prime);

        ch_next = fiber_channel_create(sizeof(int));
        f[i].ch = ch;
        f[i].ch_next = ch_next;
        f[i].prime = prime;

        fiber_spawn(filter, &f[i]);
        ch = ch_next;
    }

    exit(EXIT_SUCCESS);
}
