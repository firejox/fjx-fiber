# fjx-fiber

A thread safe stackful coroutine library in C. It would require thread library installed.

## Features

* stackful coroutines
* multithread features

## Platforms

It runs on Linux x86-64 platform. It may support other platforms in the futures.

## Sample

This is a prime-seive example.

```c
#include "fjx-fiber.h"
#include <stdio.h>
#include <stdlib.h>

struct gen_data {
    fjx_fiber_scheduler *sched;
    fjx_fiber_channel *ch;
};

struct filter_data {
    fjx_fiber_scheduler *sched;
    fjx_fiber_channel *ch;
    fjx_fiber_channel *ch_next;
    int prime;
};

void generate(void *data) {
    struct gen_data *d = (struct gen_data*)data;

    for (int i = 2; ; i++) {
        fiber_channel_send(d->sched, d->ch, &i);
    }
}

void filter(void *data) {
    struct filter_data *d = (struct filter_data*)data;
    int i;
    while (true) {
        fiber_channel_receive(d->sched, d->ch, &i);
        if (i % d->prime) {
            fiber_channel_send(d->sched, d->ch_next, &i);
        }
    }
}

int main(void) {
    fjx_fiber_scheduler *sched = NULL;
    fjx_fiber_channel *ch = NULL, *ch_next = NULL;

    sched = fiber_scheduler_create(2);
    ch = fiber_channel_create(sizeof(int));
    struct gen_data g = {.sched = sched, .ch = ch};
    struct filter_data f[103];

    fiber_spawn(sched, generate, &g);

    for (int i = 0; i < 100; i++) {
        int prime;
        fiber_channel_receive(sched, ch, &prime);
        printf("%d\n", prime);

        ch_next = fiber_channel_create(sizeof(int));
        f[i].sched = sched;
        f[i].ch = ch;
        f[i].ch_next = ch_next;
        f[i].prime = prime;

        fiber_spawn(sched, filter, &f[i]);
        ch = ch_next;
    }

    exit(EXIT_SUCCESS);
}
```

## TODO

* more platform support
* better api design
