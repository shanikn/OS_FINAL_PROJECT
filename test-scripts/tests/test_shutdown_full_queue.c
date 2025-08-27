/* ------------------------------------------------------------------ *
 *  queue is full when shutdown signal arrives; consumers must drain  *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <pthread.h>
#include <stdlib.h>
#include "plugins/sync/consumer_producer.h"

#define CAP  32
#define N    128           /* > CAP so queue is full at some point    */

static void *consumer(void *arg)
{
    consumer_producer_t *q = arg;
    while (1) {
        char *s = consumer_producer_get(q);
        if (!s) break;
        free(s);
    }
    return NULL;
}

START_TEST(graceful_shutdown_full_queue)
        {
                consumer_producer_t q;
        ck_assert_ptr_eq(consumer_producer_init(&q, CAP), NULL);

        pthread_t c;
        pthread_create(&c, NULL, consumer, &q);

        for (int i = 0; i < N; ++i)
        consumer_producer_put(&q, "X");

        /* signal no more work, then wait                             */
        consumer_producer_signal_finished(&q);
        pthread_join(c, NULL);
        consumer_producer_destroy(&q);
        }
END_TEST

        Suite *shutdown_suite(void)
{
    Suite *s = suite_create("shutdown_full");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, graceful_shutdown_full_queue);
    suite_add_tcase(s, tc);
    return s;
}