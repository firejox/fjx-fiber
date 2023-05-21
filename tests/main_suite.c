#include <check.h>
#include <stdlib.h>
#include "fjx-fiber.h"

extern Suite *test_suite(void);

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = test_suite();
    sr = srunner_create(s);

    srunner_set_tap(sr, "-");
    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    exit((number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
