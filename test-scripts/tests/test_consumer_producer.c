/* ------------------------------------------------------------------ *
 *  consumer_producer – single-thread and multi-thread edge cases     *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "plugins/sync/consumer_producer.h"

/* ---------- single-thread sanity ---------------------------------- */
START_TEST(capacity_one_put_get)
{
    consumer_producer_t q;
    ck_assert_ptr_eq(consumer_producer_init(&q, 1), NULL);

    ck_assert_ptr_eq(consumer_producer_put(&q, "A"), NULL);
    ck_assert_str_eq(consumer_producer_get(&q), "A");

    consumer_producer_destroy(&q);
}
END_TEST

/* ---------- concurrency: 2 producers, 2 consumers ----------------- */
#define RUNS 1000
static void *producer(void *arg)
{
    consumer_producer_t *q = arg;
    char buf[16];

    for (int i = 0; i < RUNS; ++i) {
        snprintf(buf, sizeof buf, "%d", i);
        consumer_producer_put(q, buf);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    consumer_producer_t *q = arg;
    int got = 0;

    while (got < RUNS) {
        char *s = consumer_producer_get(q);
        if (s) {
            ++got;
            free(s);
        }
    }
    return NULL;
}

START_TEST(two_by_two_threads)
{
    consumer_producer_t q;
    ck_assert_ptr_eq(consumer_producer_init(&q, 64), NULL);

    pthread_t p1, p2, c1, c2;
    pthread_create(&p1, NULL, producer, &q);
    pthread_create(&p2, NULL, producer, &q);
    pthread_create(&c1, NULL, consumer, &q);
    pthread_create(&c2, NULL, consumer, &q);

    pthread_join(p1, NULL); pthread_join(p2, NULL);
    consumer_producer_signal_finished(&q);   /* let consumers exit */
    pthread_join(c1, NULL); pthread_join(c2, NULL);

    consumer_producer_destroy(&q);
}
END_TEST

Suite *queue_suite(void)
{
    Suite *s = suite_create("queue");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, capacity_one_put_get);
    tcase_add_test(tc, two_by_two_threads);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(queue_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}
