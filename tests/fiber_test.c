#include <check.h>
#include "fjx-fiber.h"

extern Suite *test_suite(void);

static int shared;

static void setup(void) {
    shared = 0;
}

static void teardown(void) {

}

static void f1(void *arg) {
    int *p = (int *)arg;
    ck_assert_int_eq(*p, 0);
    *p = 1;
}

START_TEST(test_fiber_spawn_yield)
{
    ck_assert_int_eq(shared, 0);

    fiber_spawn(f1, &shared);
    fiber_yield();

    ck_assert_int_eq(shared, 1);
}
END_TEST

static void f2(void *arg) {
    (void)arg;
    struct timespec dur = { .tv_sec = 0, .tv_nsec = 1000000 };
    struct timespec ts1, ts2;

    timespec_get(&ts1, TIME_UTC);
    fiber_sleep(&dur);
    timespec_get(&ts2, TIME_UTC);

    if (ts1.tv_sec == ts2.tv_sec) {
        ck_assert_int_ge(ts2.tv_nsec - ts1.tv_nsec, dur.tv_nsec);
    } else {
        ck_assert_int_ge(ts2.tv_nsec + 1000000000l - ts1.tv_nsec, dur.tv_nsec);
    }
}

START_TEST(test_fiber_sleep)
{
    struct timespec dur = { .tv_sec = 0, .tv_nsec = 3000000 };
    struct timespec ts1, ts2;

    fiber_spawn(f2, NULL);

    timespec_get(&ts1, TIME_UTC);
    fiber_sleep(&dur);
    timespec_get(&ts2, TIME_UTC);

    if (ts1.tv_sec == ts2.tv_sec) {
        ck_assert_int_ge(ts2.tv_nsec - ts1.tv_nsec, dur.tv_nsec);
    } else {
        ck_assert_int_ge(ts2.tv_nsec + 1000000000l - ts1.tv_nsec, dur.tv_nsec);
    }
}
END_TEST

Suite *test_suite(void) {
    fiber_scheduler_init(0);

    Suite *s;
    TCase *tc_core;

    s = suite_create("Fiber");

    tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_fiber_spawn_yield);
    tcase_add_test(tc_core, test_fiber_sleep);

    suite_add_tcase(s, tc_core);

    return s;
}
