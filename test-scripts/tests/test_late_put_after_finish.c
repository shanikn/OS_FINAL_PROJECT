#include <check.h>
#include "plugins/sync/consumer_producer.h"

START_TEST(late_put_rejected)
        {
                consumer_producer_t q;
        ck_assert_ptr_eq(consumer_producer_init(&q, 4), NULL);

        consumer_producer_signal_finished(&q);
        const char *err = consumer_producer_put(&q, "Y");
        ck_assert_ptr_nonnull(err);             /* must fail          */

        consumer_producer_destroy(&q);
        }
END_TEST

        Suite *late_put_suite(void)
{
    Suite *s = suite_create("late_put");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, late_put_rejected);
    suite_add_tcase(s, tc);
    return s;
}