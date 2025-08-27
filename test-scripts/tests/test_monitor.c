/* ------------------------------------------------------------------ *
 *  Monitor – remembers signals                                       *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <pthread.h>
#include "plugins/sync/monitor.h"

START_TEST(signal_before_wait_is_caught)
{
    monitor_t m; ck_assert_int_eq(monitor_init(&m), 0);

    /* signal first … */
    monitor_signal(&m);
    /* … then wait: must return immediately */
    ck_assert_int_eq(monitor_wait(&m), 0);

    monitor_destroy(&m);
}
END_TEST


static void *waiter(void *arg)           /* helper for concurrent test */
{
    monitor_t *m = arg;
    ck_assert_int_eq(monitor_wait(m), 0);
    return NULL;
}

START_TEST(two_waiters_get_broadcast)
{
    monitor_t m; ck_assert_int_eq(monitor_init(&m), 0);
    pthread_t t1, t2;

    pthread_create(&t1, NULL, waiter, &m);
    pthread_create(&t2, NULL, waiter, &m);

    /* give threads a moment to block */
    sched_yield();
    monitor_signal(&m);                  /* broadcast */

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    monitor_destroy(&m);
}
END_TEST


Suite *monitor_suite(void)
{
    Suite *s = suite_create("monitor");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, signal_before_wait_is_caught);
    tcase_add_test(tc, two_waiters_get_broadcast);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(monitor_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}
