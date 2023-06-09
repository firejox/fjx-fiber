#include "fjx-fiber.h"
#include <stdio.h>
#include <unistd.h>

fjx_fiber_semaphore *s[5];
fjx_fiber_mutex *m;
fjx_fiber_semaphore *sync_s;

enum {
    THINKING,
    HUNGRY,
    EATING
};

int state[5];

void g(void* data) {
    struct timespec d = {.tv_sec = 5, .tv_nsec = 0};
    fiber_sleep(&d);
    fprintf(stderr, "notify\n");

    fiber_semaphore_signal(sync_s);
}

static inline int left(int i) { return (i + 4) % 5; }
static inline int right(int i) { return (i + 1) % 5; }

void test(int i) {
    if (state[i] == HUNGRY &&
        state[left(i)] != EATING &&
        state[right(i)] != EATING) {
        state[i] = EATING;
        fiber_semaphore_signal(s[i]);
    }
}

void take_forks(int i) {
    fiber_mutex_lock(m);
    state[i] = HUNGRY;
    test(i);
    fiber_mutex_unlock(m);
    fiber_semaphore_wait(s[i]);
}

void put_forks(int i) {
    fiber_mutex_lock(m);
    state[i] = THINKING;
    test(left(i));
    test(right(i));
    fiber_mutex_unlock(m);
}

void f(void* data) {
    int id = *(int*)data;
    struct timespec d = {.tv_sec = 0, .tv_nsec = 1000};

    while (1) {
        printf("ph %d wants to eat\n", id);
        take_forks(id);
        printf("ph %d pick left fork\n", id);
        printf("ph %d pick right fork\n", id);
        printf("ph %d start eat\n", id);
        put_forks(id);
        printf("ph %d put right fork\n", id);
        printf("ph %d put left fork\n", id);
        fiber_sleep(&d);
    }
}

int main(void) {
    fiber_scheduler_init(8);
    m = fiber_mutex_create();
    for (int i = 0; i < 5; i++)
        s[i] = fiber_semaphore_bound_create(0, 1);
    sync_s = fiber_semaphore_create(0);

    int id[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++) {
        fiber_spawn(f, &id[i]);
    }

    fiber_spawn(g, NULL);
    fiber_semaphore_wait(sync_s);
    fprintf(stderr, "wakeup\n");
    return 0;
}
