#include "fjx-fiber.h"
#include <stdio.h>
#include <unistd.h>

fjx_fiber_scheduler *sched;
fjx_fiber_semaphore *m;
fjx_fiber_mutex *s[5];
fjx_fiber_semaphore *sync_s;

void g(void* data) {
    sleep(5);
    fprintf(stderr, "notify\n");
    fiber_semaphore_signal(sched, sync_s);
}

void f(void* data) {
    int id = *(int*)data;
    int left = id;
    int right = (id + 1) % 5;

    while (1) {
        printf("ph %d wants to eat\n", id);
        fiber_semaphore_wait(sched, m);

        fiber_mutex_lock(sched, s[left]);
        printf("ph %d pick left fork\n", id);
        fiber_mutex_lock(sched, s[right]);
        printf("ph %d pick right fork\n", id);

        printf("ph %d start eat\n", id);

        fiber_mutex_unlock(sched, s[right]);
        printf("ph %d put right fork\n", id);
        fiber_mutex_unlock(sched, s[left]);
        printf("ph %d put left fork\n", id);

        fiber_semaphore_signal(sched, m);
    }
}

int main(void) {
    sched = fiber_scheduler_create(6);
    sleep(1);
    m = fiber_semaphore_create(4);
    for (int i = 0; i < 5; i++)
        s[i] = fiber_mutex_create();
    sync_s = fiber_semaphore_create(0);

    int id[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++) {
        fiber_spawn(sched, f, &id[i]);
    }

    fiber_spawn(sched, g, NULL);
    fprintf(stderr, "sleeping\n");
    fiber_semaphore_wait(sched, sync_s);
    fprintf(stderr, "wakeup\n");
    return 0;
}
