#include "fjx-fiber.h"
#include <stdio.h>
#include <unistd.h>

fjx_fiber_scheduler *sched;
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
    sleep(5);
    fprintf(stderr, "notify\n");
    fiber_semaphore_signal(sched, sync_s);
}

inline int left(int i) { return (i + 4) % 5; }
inline int right(int i) { return (i + 1) % 5; }

void test(int i) {
    if (state[i] == HUNGRY &&
        state[left(i)] != EATING &&
        state[right(i)] != EATING) {
        state[i] = EATING;
        fiber_semaphore_signal(sched, s[i]);
    }
}

void take_forks(int i) {
    fiber_mutex_lock(sched, m);
    state[i] = HUNGRY;
    test(i);
    fiber_mutex_unlock(sched, m);
    fiber_semaphore_wait(sched, s[i]);
}

void put_forks(int i) {
    fiber_mutex_lock(sched, m);
    state[i] = THINKING;
    test(left(i));
    test(right(i));
    fiber_mutex_unlock(sched, m);
}

void f(void* data) {
    int id = *(int*)data;

    while (1) {
        printf("ph %d wants to eat\n", id);
        take_forks(id);
        printf("ph %d pick left fork\n", id);
        printf("ph %d pick right fork\n", id);
        printf("ph %d start eat\n", id);
        put_forks(id);
        printf("ph %d put right fork\n", id);
        printf("ph %d put left fork\n", id);
    }
}

int main(void) {
    sched = fiber_scheduler_create(8);
    m = fiber_mutex_create();
    for (int i = 0; i < 5; i++)
        s[i] = fiber_semaphore_bound_create(0, 1);
    sync_s = fiber_semaphore_create(0);

    int id[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++) {
        fiber_spawn(sched, f, &id[i]);
    }

    fiber_spawn(sched, g, NULL);
    fiber_semaphore_wait(sched, sync_s);
    fprintf(stderr, "wakeup\n");
    return 0;
}
