/* ------------------------------------------------------------------ *
 *  8 producer threads hammer the queue simultaneously                *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <pthread.h>
#include <stdlib.h>
#include "plugins/sync/consumer_producer.h"

#define THREADS 8
#define MSGS    50000

static void *producer(void *arg)
{
    consumer_producer_t *q = arg;
    for (int i = 0; i < MSGS; ++i)
        consumer_producer_put(q, "X");
    return NULL;
}

START_TEST(high_frequency_load)
        {
                consumer_producer_t q;
        ck_assert_ptr_eq(consumer_producer_init(&q, 256), NULL);

        pthread_t t[THREADS];
        for (int i = 0; i < THREADS; ++i)
        pthread_create(&t[i], NULL, producer, &q);

        /* drain in this thread */
        long got = 0;
        while (got < THREADS * MSGS) {
            char *s = consumer_producer_get(&q);
            if (s) { ++got; free(s); }
        }

        for (int i = 0; i < THREADS; ++i) pthread_join(t[i], NULL);
        consumer_producer_destroy(&q);
        }
END_TEST

        Suite *high_freq_suite(void)
{
    Suite *s = suite_create("high_freq");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, high_frequency_load);
    suite_add_tcase(s, tc);
    return s;
}