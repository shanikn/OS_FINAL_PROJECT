#include <check.h>
#include <pthread.h>
#include <time.h>
#include "plugins/sync/consumer_producer.h"

static void *getter(void *arg)
{
    consumer_producer_t *q = arg;
    for (int i = 0; i < 3; ++i)         /* three blocking gets */
        free(consumer_producer_get(q));
    return NULL;
}

START_TEST(no_busy_wait)
        {
                consumer_producer_t q;
        ck_assert_ptr_eq(consumer_producer_init(&q, 4), NULL);

        pthread_t t;
        pthread_create(&t, NULL, getter, &q);

        /* give the thread time to block                             */
        struct timespec ts = { .tv_sec = 0, .tv_nsec = 100 * 1000 * 1000 };
        nanosleep(&ts, NULL);

        /* feed three items, thread will finish                      */
        for (int i = 0; i < 3; ++i)
        consumer_producer_put(&q, "X");
        consumer_producer_signal_finished(&q);
        pthread_join(t, NULL);

        /* read thread’s CPU time – should be < 5 ms                 */
        struct timespec cpu;
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu);
        ck_assert_msg(cpu.tv_nsec < 5 * 1000 * 1000,
        "getter thread busy-waited (CPU %.3f ms)",
        cpu.tv_nsec / 1e6);

        consumer_producer_destroy(&q);
        }
END_TEST

        Suite *busy_suite(void)
{
    Suite *s = suite_create("busy_wait");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, no_busy_wait);
    suite_add_tcase(s, tc);
    return s;
}